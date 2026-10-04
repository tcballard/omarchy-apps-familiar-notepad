#pragma once
#include <QIcon>
#include <QPainter>
#include <QPainterPath>
#include <QPalette>
// Small authored vector glyphs, rendered at device-independent 2x resolution.
inline QIcon toolIcon(const QString &name, const QColor &ink) {
    QPixmap pixmap(40,40); pixmap.fill(Qt::transparent); pixmap.setDevicePixelRatio(2);
    QPainter p(&pixmap); p.setRenderHint(QPainter::Antialiasing); p.setPen(QPen(ink,1.5,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
    if(name=="new") {p.drawRect(QRectF(4,2,11,15));p.drawLine(7,7,12,7);p.drawLine(7,10,12,10);p.drawLine(14,12,14,19);p.drawLine(11,15,17,15);}
    else if(name=="open") {QPainterPath a;a.moveTo(2,6);a.lineTo(2,16);a.lineTo(15,16);a.lineTo(18,7);a.lineTo(8,7);a.lineTo(6,4);a.lineTo(2,4);a.closeSubpath();p.drawPath(a);p.drawLine(3,16,6,9);p.drawLine(6,9,17,9);}
    else if(name=="save") {p.drawRect(QRectF(3,3,14,14));p.drawRect(QRectF(7,3,6,5));p.drawRect(QRectF(6,11,8,6));}
    else if(name=="find") {p.drawEllipse(QRectF(3,2,10,10));p.drawLine(12,11,17,17);}
    else if(name=="replace") {p.drawLine(3,5,16,5);p.drawLine(13,2,16,5);p.drawLine(13,8,16,5);p.drawLine(16,14,3,14);p.drawLine(6,11,3,14);p.drawLine(6,17,3,14);}
    else if(name=="wrap") {p.drawLine(3,4,16,4);p.drawLine(3,15,8,15);QPainterPath a;a.moveTo(3,9);a.lineTo(14,9);a.cubicTo(19,9,19,15,14,15);a.lineTo(11,15);p.drawPath(a);p.drawLine(14,12,11,15);p.drawLine(14,18,11,15);}
    else {if(name=="redo"){p.translate(20,0);p.scale(-1,1);}QPainterPath a;a.moveTo(4,7);a.lineTo(11,7);a.cubicTo(18,7,18,16,11,16);p.drawPath(a);p.drawLine(7,3,3,7);p.drawLine(7,11,3,7);}
    return QIcon(pixmap);
}
