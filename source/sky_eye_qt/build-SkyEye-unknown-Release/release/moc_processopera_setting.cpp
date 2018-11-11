/****************************************************************************
** Meta object code from reading C++ file 'processopera_setting.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../SkyEye/processopera_setting.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'processopera_setting.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_ProcessOpera_Setting[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      22,   21,   21,   21, 0x0a,
      47,   21,   21,   21, 0x0a,
      62,   21,   21,   21, 0x0a,
      82,   21,   21,   21, 0x0a,
     101,   21,   21,   21, 0x0a,
     118,  115,  110,   21, 0x0a,
     149,  115,  110,   21, 0x0a,
     185,  179,  110,   21, 0x0a,
     226,  179,  110,   21, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_ProcessOpera_Setting[] = {
    "ProcessOpera_Setting\0\0comboboxChanged(QString)\0"
    "allowProcess()\0forbidProcessOpen()\0"
    "forbidProcessNet()\0Detail()\0bool\0pn\0"
    "ssdtForbidprocessopen(QString)\0"
    "ssdtAllowprocessopen(QString)\0pn,pp\0"
    "wfpForbidprocessnetwork(QString,QString)\0"
    "wfpAllowprocessnetwork(QString,QString)\0"
};

void ProcessOpera_Setting::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        ProcessOpera_Setting *_t = static_cast<ProcessOpera_Setting *>(_o);
        switch (_id) {
        case 0: _t->comboboxChanged((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 1: _t->allowProcess(); break;
        case 2: _t->forbidProcessOpen(); break;
        case 3: _t->forbidProcessNet(); break;
        case 4: _t->Detail(); break;
        case 5: { bool _r = _t->ssdtForbidprocessopen((*reinterpret_cast< QString(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 6: { bool _r = _t->ssdtAllowprocessopen((*reinterpret_cast< QString(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 7: { bool _r = _t->wfpForbidprocessnetwork((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 8: { bool _r = _t->wfpAllowprocessnetwork((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        default: ;
        }
    }
}

const QMetaObjectExtraData ProcessOpera_Setting::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject ProcessOpera_Setting::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_ProcessOpera_Setting,
      qt_meta_data_ProcessOpera_Setting, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &ProcessOpera_Setting::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *ProcessOpera_Setting::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *ProcessOpera_Setting::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_ProcessOpera_Setting))
        return static_cast<void*>(const_cast< ProcessOpera_Setting*>(this));
    return QWidget::qt_metacast(_clname);
}

int ProcessOpera_Setting::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
