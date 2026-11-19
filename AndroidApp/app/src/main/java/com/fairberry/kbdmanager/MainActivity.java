package com.fairberry.kbdmanager;

import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbDeviceConnection;
import android.hardware.usb.UsbManager;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.PowerManager;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.widget.GridLayout;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;

import androidx.activity.result.ActivityResultLauncher;
import androidx.activity.result.contract.ActivityResultContracts;
import androidx.appcompat.app.AppCompatActivity;

import com.google.android.material.button.MaterialButton;
import com.google.android.material.dialog.MaterialAlertDialogBuilder;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.function.IntConsumer;

public class MainActivity extends AppCompatActivity {

    private static final String TAG = "FairberryKbd";
    private static final String ACTION_USB_PERMISSION = "com.fairberry.kbdmanager.USB_PERMISSION";
    // The grid is drawn in the physical keycap shape, not as the 5x7 scan
    // matrix; ten columns is the widest physical row and keys with a larger
    // keycap span multiple columns.
    private static final int GRID_COLUMNS = 10;
    private static final int GRID_ROWS = PhysicalLayout.ROWS.size();
    private static final int COMBO_COLUMNS = 8;
    private static final int COMBO_COL_MOD = 0;
    private static final int COMBO_COL_KEYCAP = 1;
    private static final int COMBO_COL_MOD_LINE = 2;
    private static final int COMBO_COL_OUTPUT_FIRST = 3;
    private static final int COMBO_COL_ACTION_LINE = 6;
    private static final int COMBO_COL_REMOVE = 7;
    private static final float COMBO_MOD_WEIGHT = 2f;
    private static final float COMBO_KEYCAP_WEIGHT = 3f;
    private static final float COMBO_OUTPUT_WEIGHT = 3f;
    private static final int COMBO_LINE_DP = 1;
    private static final int COMBO_REMOVE_DP = 40;
    private static final int COMBO_ROW_DP = 40;
    private static final int COMBO_HEADER_DP = 22;
    private static final int CELL_GAP_DP = 3;
    // Caterina stays in upload mode for ~8 s, so the app has to find the new
    // device instance and get permission inside that window.
    private static final int BOOTLOADER_WATCH_MS = 12000;
    // The board needs a moment to reset and re-enumerate as a new instance: looking
    // at the bus straight after 'B' finds the instance that has just gone away.
    private static final int REBOOT_SETTLE_MS = 1200;
    // The firmware idle timeout is 30 s, so the app refreshes it well inside that.
    private static final int KEEPALIVE_INTERVAL_MS = 5000;

    // Losing the keyboard while it re-enumerates as the CDC-only Caterina instance is a config
    // change, so everything a single-press flash spans must outlive one activity instance.
    private static MainActivity instance;
    private static UsbManager usbManager;
    private static PowerManager powerManager;
    private static final Handler uiHandler = new Handler(Looper.getMainLooper());
    private static final ExecutorService worker = Executors.newSingleThreadExecutor();

    private static UsbCdc cdc;
    private static KeymapClient client;
    private static KeymapData baseline;   // last state read from the keyboard
    private static KeymapData edited;     // local edits waiting to be applied
    private static ComboData comboBaseline;   // last combinations read from the keyboard
    private static ComboData comboEdited;     // local combination edits waiting to be applied
    private static int currentView;
    private static boolean bootloaderMode;
    private static boolean flashOnlyMode; // Leonardo found but protocol did not respond
    private static volatile boolean flashPending;  // flash requested while disconnected; runs when a device opens
    private static volatile boolean visible;
    private static int currentMode;

    private static byte[] hexBytes;
    private static String hexName;
    private static String status = "Not connected";

    private TextView statusText;
    private View statusDot;
    private MaterialButton connectButton;
    private MaterialButton[] modeButtons;
    private MaterialButton[] viewButtons;
    private GridLayout keyGrid;
    private final List<List<MaterialButton>> gridCells = new ArrayList<>();
    private GridLayout comboTable;
    private View modeRow;
    private TextView logText;
    private TextView logPreview;
    private MaterialButton logToggle;
    private boolean logExpanded;
    private TextView hexPathText;
    private ProgressBar flashProgress;

    private ActivityResultLauncher<Intent> hexPickerLauncher;

    private final BroadcastReceiver usbReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (ACTION_USB_PERMISSION.equals(action)) {
                UsbDevice device = intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);
                if (intent.getBooleanExtra(UsbManager.EXTRA_PERMISSION_GRANTED, false) && device != null) {
                    log("USB permission granted for " + describe(device));
                    // openDevice re-resolves the instance: the grant can name a
                    // device that has already re-enumerated away.
                    openDevice(device);
                } else {
                    log("USB permission denied");
                }
            } else if (UsbManager.ACTION_USB_DEVICE_ATTACHED.equals(action)) {
                UsbDevice device = intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);
                if (device != null && UsbCdc.hasCdc(device)
                        && (cdc == null || device.getDeviceId() != cdc.getDevice().getDeviceId())) {
                    if (!appOpen()) {
                        log("Attached " + describe(device) + " while the app is not open");
                        return;
                    }
                    log("Attached " + describe(device) + (UsbCdc.hasHid(device) ? " (sketch)" : " (bootloader)"));
                    ensurePermissionThenOpen(device);
                }
            } else if (UsbManager.ACTION_USB_DEVICE_DETACHED.equals(action)) {
                UsbDevice device = intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);
                if (device != null && cdc != null && device.getDeviceId() == cdc.getDevice().getDeviceId()) {
                    log("Keyboard detached");
                    closeConnection();
                }
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        instance = this;
        if (usbManager == null) {
            usbManager = (UsbManager) getSystemService(Context.USB_SERVICE);
            powerManager = (PowerManager) getSystemService(Context.POWER_SERVICE);
        }

        statusText = findViewById(R.id.statusText);
        statusDot = findViewById(R.id.statusDot);
        connectButton = findViewById(R.id.connectButton);
        keyGrid = findViewById(R.id.keyGrid);
        logText = findViewById(R.id.logText);
        logPreview = findViewById(R.id.logPreview);
        logToggle = findViewById(R.id.logToggle);
        hexPathText = findViewById(R.id.hexPathText);
        flashProgress = findViewById(R.id.flashProgress);

        modeButtons = new MaterialButton[]{
                findViewById(R.id.modeNormal),
                findViewById(R.id.modeSymbol),
                findViewById(R.id.modeCursor),
        };
        viewButtons = new MaterialButton[]{
                findViewById(R.id.viewKeysButton),
                findViewById(R.id.viewCombosButton),
        };
        comboTable = findViewById(R.id.comboTable);
        modeRow = findViewById(R.id.modeRow);

        buildKeyGrid();
        setMode(currentMode);

        connectButton.setOnClickListener(v -> connectToKeyboard());

        for (int m = 0; m < KeymapData.MODES; m++) {
            final int mode = m;
            modeButtons[m].setOnClickListener(v -> setMode(mode));
        }

        for (int v = 0; v < viewButtons.length; v++) {
            final int view = v;
            viewButtons[v].setOnClickListener(x -> setView(view));
        }

        findViewById(R.id.reloadButton).setOnClickListener(v -> worker.execute(this::reloadFromKeyboard));
        findViewById(R.id.applyButton).setOnClickListener(v -> worker.execute(this::applyEdits));
        findViewById(R.id.saveButton).setOnClickListener(v -> worker.execute(this::saveEeprom));
        findViewById(R.id.factoryButton).setOnClickListener(v -> worker.execute(this::factoryReset));

        findViewById(R.id.pickHexButton).setOnClickListener(v -> {
            Intent pick = new Intent(Intent.ACTION_OPEN_DOCUMENT);
            pick.addCategory(Intent.CATEGORY_OPENABLE);
            pick.setType("*/*");
            hexPickerLauncher.launch(pick);
        });
        findViewById(R.id.flashButton).setOnClickListener(v -> worker.execute(this::startFlashFlow));

        logToggle.setOnClickListener(v -> {
            logExpanded = !logExpanded;
            applyLogExpanded();
        });
        applyLogExpanded();

        hexPickerLauncher = registerForActivityResult(new ActivityResultContracts.StartActivityForResult(), result -> {
            if (result.getResultCode() != RESULT_OK || result.getData() == null) {
                return;
            }
            Uri uri = result.getData().getData();
            if (uri == null) {
                return;
            }
            worker.execute(() -> loadHexFromUri(uri));
        });

        IntentFilter filter = new IntentFilter();
        filter.addAction(ACTION_USB_PERMISSION);
        filter.addAction(UsbManager.ACTION_USB_DEVICE_ATTACHED);
        filter.addAction(UsbManager.ACTION_USB_DEVICE_DETACHED);
        if (Build.VERSION.SDK_INT >= 34) {
            // Must be EXPORTED: the USB permission result is broadcast by the
            // system, and system-originated broadcasts are not delivered to a
            // NOT_EXPORTED receiver — grants would never reach the app.
            registerReceiver(usbReceiver, filter, Context.RECEIVER_EXPORTED);
        } else {
            registerReceiver(usbReceiver, filter);
        }

        if (edited == null) {
            edited = new KeymapData();
            baseline = new KeymapData();
        }
        if (comboEdited == null) {
            comboEdited = new ComboData();
            comboBaseline = new ComboData();
        }
        setView(currentView);
        applyStatus(status);
        uiRun(() -> {
            instance.hexPathText.setText(hexName != null
                    ? "Firmware: " + hexName : "");
            instance.flashProgress.setProgress(0);
        });
    }

    private UsbDevice resolveLiveDevice(UsbDevice device) {
        // After a re-enumeration the broadcast can carry a stale instance that
        // openDevice refuses; fall back to the same vid:pid still present in the
        // device list, and return null when nothing on the bus matches so the
        // caller can react to whichever device is actually there.
        for (UsbDevice live : usbManager.getDeviceList().values()) {
            if (live.getDeviceId() == device.getDeviceId()) {
                return live;
            }
        }
        for (UsbDevice live : usbManager.getDeviceList().values()) {
            if (live.getVendorId() == device.getVendorId()
                    && live.getProductId() == device.getProductId()) {
                return live;
            }
        }
        return null;
    }

    private UsbDevice findLiveCdcDevice(int excludeDeviceId) {
        UsbDevice fallback = null;
        for (UsbDevice device : UsbCdc.cdcDevices(usbManager)) {
            if (device.getDeviceId() == excludeDeviceId) {
                continue;
            }
            if (!UsbCdc.hasHid(device)) {
                return device;
            }
            fallback = device;
        }
        // While a flash is pending, only the CDC-only Caterina instance is usable: a
        // HID-bearing device is either the pre-reboot instance that has already gone
        // away or a sketch-mode board that has not rebooted yet.
        return flashPending ? null : fallback;
    }

    /**
     * The board re-enumerates as a new instance when it resets into Caterina and
     * that instance needs its own permission grant, so the app looks for it on the
     * bus and asks for permission straight away: relying on the attach broadcast
     * alone leaves the app idle while the upload window closes.
     */
    private void watchForBootloader(int windowMs) {
        uiSetStatus("Waiting for the bootloader device (flash pending)");
        sleepQuietly(REBOOT_SETTLE_MS);
        long deadline = System.currentTimeMillis() + windowMs;
        while (System.currentTimeMillis() < deadline) {
            UsbDevice boot = findLiveCdcDevice(cdc != null ? cdc.getDevice().getDeviceId() : -1);
            if (boot != null) {
                log("Found " + describe(boot) + " while waiting for the bootloader");
                ensurePermissionThenOpen(boot);
                return;
            }
            sleepQuietly(250);
        }
        flashPending = false;
        log("Timed out waiting for the bootloader device. Press Flash again to try once more.");
    }

    private void sleepQuietly(long ms) {
        try {
            Thread.sleep(ms);
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }
    }

    private void buildKeyGrid() {
        keyGrid.removeAllViews();
        gridCells.clear();
        keyGrid.setColumnCount(GRID_COLUMNS);
        keyGrid.setRowCount(GRID_ROWS);
        for (int row = 0; row < GRID_ROWS; row++) {
            List<MaterialButton> rowCells = new ArrayList<>();
            int col = 0;
            for (PhysicalLayout.Key key : PhysicalLayout.ROWS.get(row)) {
                MaterialButton cell = (MaterialButton) getLayoutInflater()
                        .inflate(R.layout.cell_key, keyGrid, false);
                GridLayout.LayoutParams lp = new GridLayout.LayoutParams();
                lp.width = 0;
                lp.columnSpec = GridLayout.spec(col, key.weight, 1f);
                lp.rowSpec = GridLayout.spec(row);
                lp.setMargins(dp(CELL_GAP_DP), dp(CELL_GAP_DP), dp(CELL_GAP_DP), dp(CELL_GAP_DP));
                cell.setLayoutParams(lp);
                cell.setOnClickListener(v -> chooseKey(key));
                rowCells.add(cell);
                keyGrid.addView(cell);
                col += key.weight;
            }
            gridCells.add(rowCells);
        }
    }

    private void setMode(int mode) {
        currentMode = mode;
        for (int m = 0; m < KeymapData.MODES; m++) {
            modeButtons[m].setChecked(m == mode);
        }
        refreshGrid();
    }

    private void refreshGrid() {
        uiRun(() -> {
            KeymapData source = edited != null ? edited : new KeymapData();
            for (int row = 0; row < instance.gridCells.size(); row++) {
                List<PhysicalLayout.Key> keys = PhysicalLayout.ROWS.get(row);
                for (int i = 0; i < keys.size(); i++) {
                    PhysicalLayout.Key key = keys.get(i);
                    KeymapData.Entry entry = source.get(currentMode, key.col, key.row);
                    instance.gridCells.get(row).get(i).setText(KeyLabels.gridLabel(entry.value, entry.mods));
                }
            }
        });
    }

    private void chooseKey(PhysicalLayout.Key key) {
        List<KeymapData.Entry> presets = KeyLabels.presetsForMode(currentMode);
        showPicker(KeymapData.MODE_NAMES[currentMode] + " · " + key.name
                + " (col " + key.col + " row " + key.row + ")", presetLabels(presets), which -> {
            edited.set(currentMode, key.col, key.row, presets.get(which));
            refreshGrid();
        });
    }

    // ---------- combinations ----------

    private void setView(int view) {
        currentView = view;
        for (int v = 0; v < viewButtons.length; v++) {
            viewButtons[v].setChecked(v == view);
        }
        boolean keysView = view == 0;
        keyGrid.setVisibility(keysView ? View.VISIBLE : View.GONE);
        modeRow.setVisibility(keysView ? View.VISIBLE : View.GONE);
        comboTable.setVisibility(keysView ? View.GONE : View.VISIBLE);
        refreshGrid();
        refreshComboList();
    }

    private void refreshComboList() {
        uiRun(() -> {
            ComboData source = comboEdited != null ? comboEdited : new ComboData();
            List<ComboData.Combo> rows = source.rows();
            int comboLines = rows.isEmpty() ? 1 : rows.size();

            instance.comboTable.removeAllViews();
            instance.comboTable.setColumnCount(COMBO_COLUMNS);
            instance.comboTable.setRowCount(2 + comboLines);

            for (int col = 0; col < COMBO_COLUMNS; col++) {
                View cell = col == COMBO_COL_MOD_LINE || col == COMBO_COL_ACTION_LINE
                        ? instance.comboLine() : instance.comboHeader(comboColumnHeader(col));
                instance.addComboCell(0, col, cell, COMBO_HEADER_DP);
            }

            int row = 1;
            if (rows.isEmpty()) {
                TextView empty = new TextView(instance);
                empty.setTextAppearance(R.style.TextAppearance_Fairberry_Caption);
                empty.setText("No combinations. Use + below to add one, then Apply.");
                GridLayout.LayoutParams lp = new GridLayout.LayoutParams();
                lp.columnSpec = GridLayout.spec(0, COMBO_COLUMNS);
                lp.rowSpec = GridLayout.spec(row);
                lp.width = ViewGroup.LayoutParams.MATCH_PARENT;
                lp.setMargins(dp(CELL_GAP_DP), dp(CELL_GAP_DP), dp(CELL_GAP_DP), dp(CELL_GAP_DP));
                empty.setLayoutParams(lp);
                instance.comboTable.addView(empty);
            } else {
                for (ComboData.Combo combo : rows) {
                    instance.addComboRow(row++, combo);
                }
            }
            instance.addComboAddRow(row);
        });
    }

    private void addComboRow(int row, ComboData.Combo combo) {
        addComboCell(row, COMBO_COL_MOD,
                comboButton(R.layout.cell_table, KeyLabels.modifierName(combo.modBit()), () -> pickComboModifier(combo)),
                COMBO_ROW_DP);
        addComboCell(row, COMBO_COL_KEYCAP,
                comboButton(R.layout.cell_table, PhysicalLayout.nameFor(combo.col, combo.row), () -> pickComboKeycap(combo)),
                COMBO_ROW_DP);
        addComboCell(row, COMBO_COL_MOD_LINE, comboLine(), COMBO_ROW_DP);

        for (int slot = 0; slot < ComboData.OUTPUTS; slot++) {
            KeymapData.Entry output = combo.output(slot);
            final int outputSlot = slot;
            addComboCell(row, COMBO_COL_OUTPUT_FIRST + slot,
                    comboButton(R.layout.cell_table, output.isEmpty() ? "·"
                            : KeyLabels.entryLabel(output.value, output.mods),
                            () -> pickComboOutput(combo, outputSlot)),
                    COMBO_ROW_DP);
        }

        addComboCell(row, COMBO_COL_ACTION_LINE, comboLine(), COMBO_ROW_DP);
        addComboCell(row, COMBO_COL_REMOVE,
                comboButton(R.layout.cell_remove, "✕", () -> removeComboRow(combo)), COMBO_ROW_DP);
    }

    private void addComboAddRow(int row) {
        MaterialButton add = comboButton(R.layout.cell_add, "+ add combination", this::chooseNewCombo);
        GridLayout.LayoutParams lp = new GridLayout.LayoutParams();
        lp.columnSpec = GridLayout.spec(0, COMBO_COLUMNS);
        lp.rowSpec = GridLayout.spec(row);
        lp.width = ViewGroup.LayoutParams.MATCH_PARENT;
        lp.height = dp(COMBO_ROW_DP);
        lp.setMargins(dp(CELL_GAP_DP), 0, dp(CELL_GAP_DP), dp(CELL_GAP_DP));
        add.setLayoutParams(lp);
        comboTable.addView(add);
    }

    private void addComboCell(int row, int col, View cell, int heightDp) {
        float weight = comboColumnWeight(col);
        GridLayout.LayoutParams lp = new GridLayout.LayoutParams();
        lp.columnSpec = weight > 0 ? GridLayout.spec(col, 1, weight) : GridLayout.spec(col);
        lp.rowSpec = GridLayout.spec(row);
        lp.width = weight > 0 ? 0 : dp(comboFixedColumnDp(col));
        lp.height = dp(heightDp);
        int gap = dp(CELL_GAP_DP);
        lp.setMargins(col == COMBO_COL_MOD_LINE || col == COMBO_COL_ACTION_LINE ? 0 : gap, gap, gap, gap);
        cell.setLayoutParams(lp);
        comboTable.addView(cell);
    }

    private static float comboColumnWeight(int col) {
        switch (col) {
            case COMBO_COL_MOD:
                return COMBO_MOD_WEIGHT;
            case COMBO_COL_KEYCAP:
                return COMBO_KEYCAP_WEIGHT;
            case COMBO_COL_OUTPUT_FIRST:
            case COMBO_COL_OUTPUT_FIRST + 1:
            case COMBO_COL_OUTPUT_FIRST + 2:
                return COMBO_OUTPUT_WEIGHT;
            default:
                return 0f;
        }
    }

    private static int comboFixedColumnDp(int col) {
        if (col == COMBO_COL_MOD_LINE || col == COMBO_COL_ACTION_LINE) {
            return COMBO_LINE_DP;
        }
        return COMBO_REMOVE_DP;
    }

    private static String comboColumnHeader(int col) {
        switch (col) {
            case COMBO_COL_MOD:
                return "Mod";
            case COMBO_COL_KEYCAP:
                return "Keycap";
            case COMBO_COL_REMOVE:
                return "Del";
            default:
                return "Out " + (col - COMBO_COL_OUTPUT_FIRST + 1);
        }
    }

    /* Cell templates: the table and grid are filled from code, so each cell type needs one
       reusable style, and a layout file is the only place a style can be applied to a view
       created at runtime. */
    private MaterialButton comboButton(int layoutRes, String label, Runnable onPick) {
        MaterialButton cell = (MaterialButton) getLayoutInflater().inflate(layoutRes, comboTable, false);
        cell.setText(label);
        cell.setOnClickListener(v -> onPick.run());
        return cell;
    }

    private TextView comboHeader(String label) {
        TextView cell = (TextView) getLayoutInflater().inflate(R.layout.cell_header, comboTable, false);
        cell.setText(label);
        return cell;
    }

    private View comboLine() {
        View line = new View(this);
        line.setBackgroundColor(getColor(R.color.outline_strong));
        return line;
    }

    private int dp(int value) {
        return Math.round(value * getResources().getDisplayMetrics().density);
    }

    // The pair must stay unique, so a picker only offers the values that are not
    // already taken by another row.
    private void pickComboModifier(ComboData.Combo combo) {
        List<String> labels = new ArrayList<>();
        List<Integer> mods = new ArrayList<>();
        for (int bit = 0; bit < 8; bit++) {
            int mod = 1 << bit;
            int index = comboEdited.indexOf(mod, combo.col, combo.row);
            if (index >= 0 && !comboEdited.get(index).equals(combo)) {
                continue;
            }
            labels.add(KeyLabels.modifierName(bit));
            mods.add(mod);
        }
        showPicker("Modifier for " + PhysicalLayout.nameFor(combo.col, combo.row), labels, which ->
                updateCombo(combo, new ComboData.Combo(mods.get(which), combo.col, combo.row, combo.outputs)));
    }

    private void pickComboKeycap(ComboData.Combo combo) {
        List<String> labels = new ArrayList<>();
        List<PhysicalLayout.Key> keys = new ArrayList<>();
        for (PhysicalLayout.Key key : PhysicalLayout.allKeys()) {
            int index = comboEdited.indexOf(combo.mod, key.col, key.row);
            if (index >= 0 && !comboEdited.get(index).equals(combo)) {
                continue;
            }
            labels.add(key.name);
            keys.add(key);
        }
        showPicker(KeyLabels.modifierName(combo.modBit()) + " + keycap", labels, which -> {
            PhysicalLayout.Key key = keys.get(which);
            updateCombo(combo, new ComboData.Combo(combo.mod, key.col, key.row, combo.outputs));
        });
    }

    private void pickComboOutput(ComboData.Combo combo, int slot) {
        List<KeymapData.Entry> presets = KeyLabels.outputPresets();
        showPicker("Output " + (slot + 1) + " for " + KeyLabels.comboLabel(combo),
                presetLabels(presets), which -> {
                    KeymapData.Entry picked = presets.get(which);
                    if (picked.isEmpty()) {
                        boolean otherOutput = false;
                        for (int i = 0; i < ComboData.OUTPUTS; i++) {
                            if (i != slot && !combo.output(i).isEmpty()) {
                                otherOutput = true;
                            }
                        }
                        if (!otherOutput) {
                            Toast.makeText(this, "A combination needs at least one output key",
                                    Toast.LENGTH_SHORT).show();
                            return;
                        }
                    }
                    List<KeymapData.Entry> outputs = new ArrayList<>(combo.outputs);
                    outputs.set(slot, picked);
                    updateCombo(combo, new ComboData.Combo(combo.mod, combo.col, combo.row, outputs));
                });
    }

    private void chooseNewCombo() {
        if (comboEdited.rows().size() >= ComboData.MAX) {
            Toast.makeText(this, "Combination table is full (" + ComboData.MAX + " rows)",
                    Toast.LENGTH_SHORT).show();
            return;
        }
        List<String> modifierNames = new ArrayList<>();
        for (int bit = 0; bit < 8; bit++) {
            modifierNames.add(KeyLabels.modifierName(bit));
        }
        showPicker("New combination — modifier", modifierNames, modifierIndex -> {
            final int mod = 1 << modifierIndex;
            List<String> keyNames = new ArrayList<>();
            List<PhysicalLayout.Key> keys = new ArrayList<>();
            for (PhysicalLayout.Key key : PhysicalLayout.allKeys()) {
                if (comboEdited.indexOf(mod, key.col, key.row) >= 0) {
                    continue;
                }
                keyNames.add(key.name);
                keys.add(key);
            }
            showPicker(KeyLabels.modifierName(modifierIndex) + " + keycap", keyNames, keyIndex -> {
                PhysicalLayout.Key key = keys.get(keyIndex);
                List<KeymapData.Entry> presets = KeyLabels.outputPresets();
                showPicker(KeyLabels.modifierName(modifierIndex) + " + " + key.name + " -> output",
                        presetLabels(presets), outputIndex -> {
                            KeymapData.Entry picked = presets.get(outputIndex);
                            if (picked.isEmpty()) {
                                Toast.makeText(this, "Pick an output key for the new combination",
                                        Toast.LENGTH_SHORT).show();
                                return;
                            }
                            List<KeymapData.Entry> outputs = new ArrayList<>();
                            for (int i = 0; i < ComboData.OUTPUTS; i++) {
                                outputs.add(i == 0 ? picked : KeymapData.EMPTY);
                            }
                            comboEdited.add(new ComboData.Combo(mod, key.col, key.row, outputs));
                            refreshComboList();
                        });
            });
        });
    }

    private void removeComboRow(ComboData.Combo combo) {
        comboEdited.remove(combo.mod, combo.col, combo.row);
        refreshComboList();
    }

    private void updateCombo(ComboData.Combo oldCombo, ComboData.Combo newCombo) {
        if (oldCombo.mod != newCombo.mod || oldCombo.col != newCombo.col || oldCombo.row != newCombo.row) {
            // A row is identified by its modifier + keycap pair, so changing either
            // half moves it to a different slot.
            comboEdited.remove(oldCombo.mod, oldCombo.col, oldCombo.row);
        }
        comboEdited.add(newCombo);
        refreshComboList();
    }

    private static List<String> presetLabels(List<KeymapData.Entry> presets) {
        List<String> labels = new ArrayList<>();
        for (KeymapData.Entry entry : presets) {
            labels.add(KeyLabels.entryLabel(entry.value, entry.mods));
        }
        return labels;
    }

    private void showPicker(String title, List<String> labels, IntConsumer onPicked) {
        if (client == null) {
            Toast.makeText(this, "Connect to the keyboard first", Toast.LENGTH_SHORT).show();
            return;
        }
        new MaterialAlertDialogBuilder(this)
                .setTitle(title)
                .setItems(labels.toArray(new String[0]), (dialog, which) -> onPicked.accept(which))
                .show();
    }

    // ---------- connection handling ----------

    private void connectToKeyboard() {
        List<UsbDevice> candidates = UsbCdc.cdcDevices(usbManager);
        if (candidates.isEmpty()) {
            log("No USB CDC device found. Is the keyboard connected?");
            return;
        }
        UsbDevice chosen = null;
        for (UsbDevice device : candidates) {
            log("Found " + describe(device));
            if (chosen == null || isKnownKeyboard(device)) {
                if (chosen == null || (isKnownKeyboard(device) && !isKnownKeyboard(chosen))) {
                    chosen = device;
                }
            }
        }
        ensurePermissionThenOpen(chosen);
    }

    private static boolean isKnownKeyboard(UsbDevice device) {
        int vid = device.getVendorId();
        return vid == 0x2310 || vid == 0x2341 || vid == 0x2A03;
    }

    private static String describe(UsbDevice device) {
        return String.format("%04x:%04x", device.getVendorId(), device.getProductId())
                + (device.getProductName() != null ? " " + device.getProductName() : "");
    }

    /**
     * USB permission is only asked for when the app is open. A flash started from
     * the foreground still counts as open: the permission dialog pauses the
     * activity, and the bootloader instance has to be granted within the short
     * upload window that the flash flow is waiting for.
     */
    private boolean appOpen() {
        return visible || flashPending;
    }

    private final Runnable keepAliveLoop = new Runnable() {
        @Override
        public void run() {
            if (visible && screenOn() && client != null && !bootloaderMode) {
                KeymapClient target = client;
                worker.execute(() -> {
                    try {
                        target.keepAlive();
                        target.drainPending(500);
                    } catch (Exception e) {
                        log("Keepalive failed: " + e.getMessage());
                    }
                });
            }
            if (visible) {
                uiHandler.postDelayed(this, KEEPALIVE_INTERVAL_MS);
            }
        }
    };

    private boolean screenOn() {
        return powerManager.isInteractive();
    }

    private void ensurePermissionThenOpen(UsbDevice device) {
        if (device == null) {
            log("No USB CDC device found. Is the keyboard connected?");
            return;
        }
        UsbDevice live = resolveLiveDevice(device);
        if (live == null) {
            log("Ignoring " + describe(device) + ": that instance is no longer on the bus");
            if (flashPending) {
                watchForBootloader(BOOTLOADER_WATCH_MS);
            }
            return;
        }
        if (usbManager.hasPermission(live)) {
            log("Permission already held for " + describe(live) + ", no dialog to confirm");
            openDevice(live);
            return;
        }
        if (!appOpen()) {
            log("Keyboard present but the app is not open — permission is only requested when the app is open");
            return;
        }
        // Permissions are per device instance, so the dialog has to be confirmed
        // for the bootloader instance before its upload window closes.
        log("Requesting USB permission for " + describe(live) + " — confirm the dialog");
        // Must be MUTABLE: system_server attaches EXTRA_DEVICE and
        // EXTRA_PERMISSION_GRANTED when it fires the PendingIntent; an immutable
        // one arrives with no extras, so every grant reads as "denied".
        // FLAG_MUTABLE requires an explicit Intent on API 34+, hence setPackage.
        int flags = PendingIntent.FLAG_UPDATE_CURRENT
                | (Build.VERSION.SDK_INT >= 31 ? PendingIntent.FLAG_MUTABLE : 0);
        Intent permissionIntent = new Intent(ACTION_USB_PERMISSION);
        permissionIntent.setPackage(getPackageName());
        PendingIntent intent = PendingIntent.getBroadcast(this, 0, permissionIntent, flags);
        usbManager.requestPermission(live, intent);
    }

    private void openDevice(UsbDevice device) {
        worker.execute(() -> {
            UsbDevice live = resolveLiveDevice(device);
            if (cdc != null && live != null && live.getDeviceId() == cdc.getDevice().getDeviceId()) {
                log("Already connected to " + describe(live));
                uiSetStatus(bootloaderMode ? "Bootloader mode " + describe(live) : "Connected " + describe(live));
                return;
            }
            if (live == null) {
                // A grant can arrive for an instance that has already gone away:
                // the board re-enumerates when it resets into Caterina. Work
                // against whichever CDC device is on the bus now instead.
                UsbDevice present = findLiveCdcDevice(-1);
                if (present == null) {
                    log("Ignoring grant for " + describe(device) + ": that instance is gone and nothing is on the bus");
                    if (flashPending) {
                        watchForBootloader(BOOTLOADER_WATCH_MS);
                    }
                    return;
                }
                log("Grant for " + describe(device) + " (gone); using the live " + describe(present));
                ensurePermissionThenOpen(present);
                return;
            }
            closeConnection();
            try {
                UsbDeviceConnection connection = null;
                // Caterina re-enumerates as a new instance; a null open right
                // after attach is a race, so retry.
                for (int attempt = 0; attempt < 4 && connection == null; attempt++) {
                    connection = usbManager.openDevice(live);
                    if (connection == null) {
                        sleepQuietly(400);
                    }
                }
                if (connection == null) {
                    log("Could not open the USB device");
                    if (flashPending) {
                        watchForBootloader(BOOTLOADER_WATCH_MS);
                    }
                    return;
                }
                cdc = new UsbCdc(live, connection);
                bootloaderMode = !UsbCdc.hasHid(live);

                if (bootloaderMode) {
                    client = null;
                    uiSetStatus("Bootloader mode " + describe(live));
                    log("Caterina bootloader detected — ready to flash");
                    if (flashPending || hexBytes != null) {
                        startFlashFlow();
                    }
                    return;
                }

                try {
                    client = new KeymapClient(cdc, this::log);
                    client.sync();
                    KeymapClient.Info info = client.readInfo();
                    uiSetStatus("Connected " + describe(live) + " fw " + info.firmware);
                    log("Keyboard protocol v" + info.protocol + ", storage " + info.storage);
                    reloadFromKeyboard();
                } catch (Exception e) {
                    if (flashPending) {
                        log("Protocol silent on " + describe(live) + " while a flash is pending");
                        startFlashFlow();
                        return;
                    }
                    if (isKnownKeyboard(live)) {
                        // A Leonardo running stock firmware still answers
                        // CDC line coding, so flashing stays available even
                        // though the keymap protocol is silent.
                        flashOnlyMode = true;
                        client = null;
                        uiSetStatus("Found " + describe(live) + " — flash-only (protocol silent)");
                        log("Keyboard did not answer the keymap protocol; flashing is still available.");
                        log("Press Flash and double-press the reset button when asked; if the running firmware implements the Arduino core CDC hook, the app triggers the bootloader itself via the 1200-baud touch.");
                    } else {
                        log("Connect failed: " + e.getMessage());
                        closeConnection();
                        return;
                    }
                }
                if (flashPending) {
                    // flashPending is only consumed when a flash actually starts, so a
                    // sketch-mode device keeps the request alive: 'B' has not taken effect
                    // yet, so the app waits for the board to re-enumerate.
                    startFlashFlow();
                }
            } catch (Exception e) {
                log("Connect failed: " + e.getMessage());
                closeConnection();
            }
        });
    }

    private void closeConnection() {
        if (cdc != null) {
            try {
                cdc.close();
            } catch (Exception ignored) {
            }
            cdc = null;
        }
        client = null;
        bootloaderMode = false;
        flashOnlyMode = false;
        baseline = new KeymapData();
        comboBaseline = new ComboData();
        uiSetStatus("Not connected");
        refreshGrid();
        refreshComboList();
    }

    private KeymapClient requireClient() throws Exception {
        if (client == null) {
            throw new IllegalStateException("keyboard is not connected in sketch mode");
        }
        return client;
    }

    // ---------- keymap and combination actions ----------

    private void reloadFromKeyboard() {
        try {
            KeymapClient c = requireClient();
            KeymapData freshKeymap = c.readKeymap();
            ComboData freshCombos = c.readCombos();
            runOnUiThread(() -> {
                baseline = freshKeymap;
                edited = freshKeymap.copy();
                comboBaseline = freshCombos;
                comboEdited = freshCombos.copy();
                refreshGrid();
                refreshComboList();
                log("Loaded keymap and combinations from keyboard");
            });
        } catch (Exception e) {
            log("Reload failed: " + e.getMessage());
        }
    }

    private void applyEdits() {
        try {
            KeymapClient c = requireClient();
            List<KeymapData.Change> changes = edited.changesAgainst(baseline);
            if (!changes.isEmpty()) {
                c.applyChanges(changes);
                baseline = edited.copy();
                log("Applied " + changes.size() + " key change(s)");
            }

            // Removals before additions: a combination row is identified by its
            // modifier + keycap pair, which must stay unique, so a row that changed
            // identity has to free its pair before the new pair is added.
            List<ComboData.Combo> removals = comboEdited.removalsAgainst(comboBaseline);
            List<ComboData.Combo> additions = comboEdited.additionsAgainst(comboBaseline);
            for (ComboData.Combo combo : removals) {
                c.removeCombo(combo);
            }
            for (ComboData.Combo combo : additions) {
                c.addCombo(combo);
            }
            if (!removals.isEmpty() || !additions.isEmpty()) {
                comboBaseline = comboEdited.copy();
                log("Applied " + (removals.size() + additions.size())
                        + " combination change(s)");
            }
        } catch (Exception e) {
            log("Apply failed: " + e.getMessage());
        }
    }

    private void saveEeprom() {
        try {
            KeymapClient c = requireClient();
            applyEdits();
            c.writeEeprom();
            log("Keymap written to EEPROM");
            c.writeCombosToEEPROM();
            log("Combinations written to EEPROM");
        } catch (Exception e) {
            log("Save failed: " + e.getMessage());
        }
    }

    private void factoryReset() {
        try {
            KeymapClient c = requireClient();
            c.loadFactoryDefaults();
            c.loadComboDefaults();
            reloadFromKeyboard();
            log("Factory defaults loaded (not yet saved to EEPROM)");
        } catch (Exception e) {
            log("Factory reset failed: " + e.getMessage());
        }
    }

    private void rebootToBootloader() {
        try {
            if (cdc == null || bootloaderMode) {
                log("Keyboard is not in sketch mode");
                return;
            }
            applyEditsQuietly();
            requireClient().enterBootloader();
            log("Keyboard rebooting into the bootloader...");
            closeConnection();
            watchForBootloader(BOOTLOADER_WATCH_MS);
        } catch (Exception e) {
            log("Bootloader reboot failed: " + e.getMessage());
        }
    }

    private void applyEditsQuietly() {
        try {
            applyEdits();
        } catch (Exception ignored) {
        }
    }

    // ---------- firmware flashing ----------

    private void loadHexFromUri(Uri uri) {
        try {
            StringBuilder sb = new StringBuilder();
            try (BufferedReader reader = new BufferedReader(
                    new InputStreamReader(getContentResolver().openInputStream(uri)))) {
                String line;
                while ((line = reader.readLine()) != null) {
                    sb.append(line).append('\n');
                }
            }
            IntelHex parsed = IntelHex.parse(sb.toString());
            if (parsed.isEmpty()) {
                log("Hex file has no flashable data (bootloader region only?)");
                return;
            }
            hexBytes = sb.toString().getBytes();
            hexName = uri.getLastPathSegment();
            uiHexSelected(parsed);
            log("Loaded " + hexName + ", highest address 0x"
                    + Integer.toHexString(parsed.highestAddress()));
        } catch (Exception e) {
            log("Could not read hex file: " + e.getMessage());
        }
    }

    private void startFlashFlow() {
        if (hexBytes == null) {
            uiToast("Select a .hex file first");
            return;
        }
        if (cdc == null) {
            log("Flash: not connected yet, connecting first...");
            flashPending = true;
            connectToKeyboard();
            return;
        }
        if (bootloaderMode) {
            flashNow();
            return;
        }
        flashPending = true;
        if (client != null) {
            log("Rebooting the keyboard into the bootloader to flash...");
            rebootToBootloader();
            return;
        }
        // Protocol-silent Leonardo: no 'B' reboot command available, so try the
        // 1200-baud DTR touch the Arduino core honours in the running sketch.
        log("Trying 1200-baud bootloader touch...");
        boolean touched = cdc.setLineCoding(1200);
        touched &= cdc.setControlLineState(true, true);
        touched &= cdc.setControlLineState(false, false);
        if (!touched) {
            log("Device did not accept CDC line commands");
        }
        log("Upload mode entered: confirm the permission dialog straight away, the flash runs on its own.");
        watchForBootloader(BOOTLOADER_WATCH_MS);
    }

    private void uiToast(String message) {
        uiRun(() -> Toast.makeText(instance, message, Toast.LENGTH_SHORT).show());
    }

    private void flashNow() {
        flashPending = false;
        uiSetStatus("Flashing " + hexName);
        uiFlashProgress(true, 0);
        worker.execute(() -> {
            try {
                IntelHex hex = IntelHex.parse(new String(hexBytes));
                CaterinaFlasher flasher = new CaterinaFlasher(cdc);
                flasher.flash(hex, (percent, message) -> runOnUiThread(() -> {
                    uiFlashProgress(true, percent);
                    log(message);
                }));
                uiFlashProgress(false, 0);
                log("Flash complete: the keyboard restarts on its own");
            } catch (Exception e) {
                uiFlashProgress(false, 0);
                log("Flash failed: " + e.getMessage());
            }
        });
    }

    // ---------- UI helpers (worker-safe) ----------

    private static void uiRun(Runnable action) {
        uiHandler.post(() -> {
            if (instance != null) {
                action.run();
            }
        });
    }

    private void log(String message) {
        Log.i(TAG, message);
        uiRun(() -> {
            String current = instance.logText.getText().toString();
            if (current.length() > 4000) {
                current = current.substring(current.length() - 2500);
            }
            instance.logText.setText(current + message + "\n");
            instance.logPreview.setText(message);
        });
    }

    private void applyLogExpanded() {
        logToggle.setIconResource(logExpanded ? R.drawable.caret_up : R.drawable.caret_down);
        logText.setVisibility(logExpanded ? View.VISIBLE : View.GONE);
        logPreview.setVisibility(logExpanded ? View.GONE : View.VISIBLE);
    }

    private void uiSetStatus(String text) {
        log("status: " + text);
        status = text;
        uiRun(() -> instance.applyStatus(text));
    }

    private void applyStatus(String text) {
        String lower = text.toLowerCase();
        int color;
        if (lower.contains("bootloader") || lower.contains("waiting") || lower.contains("flashing")) {
            color = getColor(R.color.state_pending);
        } else if (lower.startsWith("connected")) {
            color = getColor(R.color.state_connected);
        } else {
            color = getColor(R.color.state_idle);
        }
        statusDot.getBackground().mutate().setTint(color);
        statusText.setTextColor(color);
        statusText.setText(text);
    }

    private void uiHexSelected(IntelHex hex) {
        uiRun(() -> instance.hexPathText.setText("Firmware: " + hexName
                + " (top app address 0x" + Integer.toHexString(hex.highestAddress()) + ")"));
    }

    private void uiFlashProgress(boolean show, int percent) {
        uiRun(() -> {
            instance.flashProgress.setVisibility(show ? View.VISIBLE : View.GONE);
            instance.flashProgress.setProgress(percent);
        });
    }

    // Permission is only requested while the app is open, so a keyboard that
    // appeared while the activity was in the background is picked up here when it
    // comes back to the foreground. The receiver stays registered meanwhile: the
    // grant itself arrives while the permission dialog has the activity paused.
    @Override
    protected void onResume() {
        super.onResume();
        instance = this;
        visible = true;
        if (cdc != null && resolveLiveDevice(cdc.getDevice()) == null) {
            // A board that went through an idle sleep detaches and re-attaches, so a
            // connection kept across that gap is stale.
            closeConnection();
        }
        if (cdc == null) {
            for (UsbDevice device : UsbCdc.cdcDevices(usbManager)) {
                if (isKnownKeyboard(device)) {
                    ensurePermissionThenOpen(device);
                    break;
                }
            }
        } else {
            uiSetStatus((bootloaderMode ? "Bootloader mode " : "Connected ") + describe(cdc.getDevice()));
        }
        refreshGrid();
        uiHandler.removeCallbacks(keepAliveLoop);
        uiHandler.postDelayed(keepAliveLoop, KEEPALIVE_INTERVAL_MS);
    }

    @Override
    protected void onPause() {
        super.onPause();
        visible = false;
        uiHandler.removeCallbacks(keepAliveLoop);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        uiHandler.removeCallbacks(keepAliveLoop);
        try {
            unregisterReceiver(usbReceiver);
        } catch (Exception ignored) {
        }
        if (instance == this) {
            instance = null;
        }
        // A recreation must not end the connection a flash depends on: only the app
        // actually finishing does.
        if (isFinishing()) {
            closeConnection();
        }
    }
}