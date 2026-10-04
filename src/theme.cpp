#include "theme.h"
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QPalette>
#include <QRegularExpression>
#include <QStandardPaths>

QString themePath() {
    QString base = qEnvironmentVariable("XDG_CONFIG_HOME");
    if (!QDir::isAbsolutePath(base)) base = QDir::homePath() + "/.config";
    return base + "/omarchy/current/theme/colors.toml";
}
void applyTheme(QApplication *app) {
    QMap<QString, QColor> colours{{"background", QColor("#20242b")}, {"foreground", QColor("#e6e8eb")},
        {"lighter_background", QColor("#2c323b")}, {"selection", QColor("#435a50")}};
    QFile file(themePath());
    if (file.open(QIODevice::ReadOnly)) {
        // Read only the top-level quoted colour assignments used by Omarchy.
        const auto text = QString::fromUtf8(file.read(64 * 1024));
        const QRegularExpression expression(R"re(^\s*([a-z_]+)\s*=\s*"(#[0-9a-fA-F]{6})"\s*(?:#.*)?$)re", QRegularExpression::MultilineOption);
        auto matches = expression.globalMatch(text);
        while (matches.hasNext()) { auto match = matches.next(); if (colours.contains(match.captured(1))) colours[match.captured(1)] = QColor(match.captured(2)); }
    }
    QPalette palette;
    for (auto role : {QPalette::Window, QPalette::Base}) palette.setColor(role, colours["background"]);
    palette.setColor(QPalette::Button, colours["lighter_background"]);
    for (auto role : {QPalette::WindowText, QPalette::Text, QPalette::ButtonText, QPalette::HighlightedText}) palette.setColor(role, colours["foreground"]);
    palette.setColor(QPalette::Highlight, colours["selection"]);
    palette.setColor(QPalette::PlaceholderText, QColor("#a0a8ad"));
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor("#80858c"));
    if (app->palette() != palette) app->setPalette(palette);
    // Raised/sunken edges echo classic Paint without rasterising the interface.
    const QString panel = colours["lighter_background"].name();
    const QString base = colours["background"].name();
    const QString edge = colours["lighter_background"].lighter(155).name();
    const QString shadow = colours["background"].darker(160).name();
    const QString selection = colours["selection"].name();
    const QString foreground = colours["foreground"].name();
    const QString css = QString(R"css(
        QMainWindow { background:%1; }
        QMenuBar { background:%1; padding:3px 8px; border-bottom:1px solid %4; }
        QMenuBar::item { padding:5px 12px; }
        QMenuBar::item:selected { background:%5; }
        QToolBar { background:%2; spacing:4px; padding:8px 12px; border-top:1px solid %3; border-bottom:1px solid %4; }
        QToolBar::separator { background:%3; width:1px; margin:9px 10px; }
        QToolButton { color:%6; min-width:48px; padding:5px 8px; border-radius:2px; border:1px solid transparent; }
        QToolButton:hover { background:%5; border-color:%3; }
        QToolButton:checked { background:%5; border:1px solid %3; border-bottom:2px solid #e8cf87; }
        QToolButton:focus { border:1px dashed #e8cf87; }
        QToolButton:pressed { background:%1; border:1px solid %4; }
        QToolButton#qt_toolbar_ext_button { min-width:16px; padding:0; }
        QWidget#documentHeader { background:%2; border:1px solid %3; border-top:2px solid #e8cf87; border-bottom:none; }
        QLabel#documentTitle { color:%6; font-size:13px; font-weight:600; }
        QLabel#documentPath { color:%6; font-size:11px; }
        QPlainTextEdit#paper { background:#faf9f3; color:#263932; selection-background-color:#ceded2; selection-color:#172b22; border:1px solid #626b64; }
        QWidget#searchPanel { background:%2; border:1px solid %3; border-top:none; }
        QLabel#notice { background:#e8cf87; color:#29281f; padding:10px; }
        QLineEdit { background:%1; color:%6; padding:7px 9px; border:1px solid %3; border-radius:2px; }
        QLineEdit:focus { border-color:#e8cf87; }
        QPushButton { padding:7px 12px; border:1px solid %3; border-radius:2px; background:%2; }
        QPushButton:hover { background:%5; }
        QPushButton:focus { border:1px solid #e8cf87; }
        QCheckBox { padding:3px; }
        QStatusBar { background:%2; border-top:1px solid %3; padding:6px 12px; font-size:11px; }
        QStatusBar::item { border:none; }
    )css").arg(base, panel, edge, shadow, selection, foreground);
    if (app->styleSheet() != css) app->setStyleSheet(css);
}
