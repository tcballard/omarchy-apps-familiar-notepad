# Architecture

This first preview uses C++17 and Qt6 Widgets to match Familiar Paint. QPlainTextEdit supplies native text input, selection, undo and accessibility rather than implementing an editor on a custom canvas.

- `storage` owns strict decoding, newline normalization, encoding, hashes, cooperative file locks and atomic writes.
- `recovery` owns version-1 private JSON snapshots and per-instance locks. Recovery is separate from the original file.
- `editor` owns the native plain-text widget and grouped replacement. Text export uses `toRawText` and converts structural paragraph separators to LF, preserving non-breaking spaces that `toPlainText` normalizes.
- `window` owns document identity, modified/format flags, dialogs, actions and UI state.
- `main` owns CLI parsing and application startup. Inspection uses QCoreApplication and does not need a display server; the executable remains linked to Qt Widgets.
- `theme` reads a bounded Omarchy colour file and applies native palette/style rules, retaining a fixed paper surface.

Load/save work runs through QtConcurrent. The window owns one QFutureWatcher; it disables file actions and editing until the request completes. Closing while busy is refused. Completion commits the result on the GUI thread; the destructor waits for the worker. There is no queued second document request to race an earlier completion. Explicit discard is only finalized when replacement succeeds or the window closes.

Recovery snapshots are debounced on the UI thread, so a large snapshot can briefly delay input. The 8 MiB input/output ceiling bounds work; it is a preview limit, not a large-file performance claim. There is no telemetry, account, subprocess engine or application network access.
