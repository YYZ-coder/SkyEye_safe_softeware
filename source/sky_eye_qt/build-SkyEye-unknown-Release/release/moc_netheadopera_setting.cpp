/****************************************************************************
** Meta object code from reading C++ file 'netheadopera_setting.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../SkyEye/netheadopera_setting.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'netheadopera_setting.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_NetHeadOpera_Setting[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      22,   21,   21,   21, 0x0a,
      38,   21,   21,   21, 0x0a,
      53,   21,   21,   21, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_NetHeadOpera_Setting[] = {
    "NetHeadOpera_Setting\0\0forbidNethead()\0"
    "allowNethead()\0showDetail()\0"
};

void NetHeadOpera_Setting::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        NetHeadOpera_Setting *_t = static_cast<NetHeadOpera_Setting *>(_o);
        switch (_id) {
        case 0: _t->forbidNethead(); break;
        case 1: _t->allowNethead(); break;
        case 2: _t->showDetail(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData NetHeadOpera_Setting::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject NetHeadOpera_Setting::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_NetHeadOpera_Setting,
      qt_meta_data_NetHeadOpera_Setting, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &NetHeadOpera_Setting::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *NetHeadOpera_Setting::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *NetHeadOpera_Setting::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_NetHeadOpera_Setting))
        return static_cast<void*>(const_cast< NetHeadOpera_Setting*>(this));
    return QWidget::qt_metacast(_clname);
}

int NetHeadOpera_Setting::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
