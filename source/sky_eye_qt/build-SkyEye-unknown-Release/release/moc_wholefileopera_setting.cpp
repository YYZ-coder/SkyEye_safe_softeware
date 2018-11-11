/****************************************************************************
** Meta object code from reading C++ file 'wholefileopera_setting.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../SkyEye/wholefileopera_setting.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'wholefileopera_setting.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_waitForBack[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      13,   12,   12,   12, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_waitForBack[] = {
    "waitForBack\0\0haveData()\0"
};

void waitForBack::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        waitForBack *_t = static_cast<waitForBack *>(_o);
        switch (_id) {
        case 0: _t->haveData(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData waitForBack::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject waitForBack::staticMetaObject = {
    { &QThread::staticMetaObject, qt_meta_stringdata_waitForBack,
      qt_meta_data_waitForBack, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &waitForBack::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *waitForBack::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *waitForBack::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_waitForBack))
        return static_cast<void*>(const_cast< waitForBack*>(this));
    return QThread::qt_metacast(_clname);
}

int waitForBack::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QThread::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    return _id;
}

// SIGNAL 0
void waitForBack::haveData()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}
static const uint qt_meta_data_WholeFileOpera_Setting[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      18,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      24,   23,   23,   23, 0x0a,
      35,   23,   23,   23, 0x0a,
      53,   23,   23,   23, 0x0a,
      62,   23,   23,   23, 0x0a,
      82,   23,   23,   23, 0x0a,
     104,   23,   23,   23, 0x0a,
     127,   23,   23,   23, 0x0a,
     150,   23,   23,   23, 0x0a,
     174,   23,   23,   23, 0x0a,
     195,   23,   23,   23, 0x0a,
     217,   23,   23,   23, 0x0a,
     241,   23,   23,   23, 0x0a,
     266,   23,   23,   23, 0x0a,
     288,   23,   23,   23, 0x0a,
     311,   23,   23,   23, 0x0a,
     332,   23,   23,   23, 0x0a,
     356,   23,   23,   23, 0x0a,
     381,   23,   23,   23, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_WholeFileOpera_Setting[] = {
    "WholeFileOpera_Setting\0\0clearCkb()\0"
    "gotReadFromBack()\0addBtn()\0"
    "cobChanged(QString)\0func_ckb_ReadDir(int)\0"
    "func_ckb_ReadFile(int)\0func_ckb_WriteDir(int)\0"
    "func_ckb_WriteFile(int)\0func_ckb_AllDir(int)\0"
    "func_ckb_AllFile(int)\0func_ckb_DeleteDir(int)\0"
    "func_ckb_DeleteFile(int)\0func_ckb_MoveDir(int)\0"
    "func_ckb_MoveFile(int)\0func_ckb_newDir(int)\0"
    "func_ckb_RenameDir(int)\0"
    "func_ckb_RenameFile(int)\0"
    "func_lne_textChanged(QString)\0"
};

void WholeFileOpera_Setting::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        WholeFileOpera_Setting *_t = static_cast<WholeFileOpera_Setting *>(_o);
        switch (_id) {
        case 0: _t->clearCkb(); break;
        case 1: _t->gotReadFromBack(); break;
        case 2: _t->addBtn(); break;
        case 3: _t->cobChanged((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 4: _t->func_ckb_ReadDir((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: _t->func_ckb_ReadFile((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 6: _t->func_ckb_WriteDir((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 7: _t->func_ckb_WriteFile((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 8: _t->func_ckb_AllDir((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 9: _t->func_ckb_AllFile((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 10: _t->func_ckb_DeleteDir((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 11: _t->func_ckb_DeleteFile((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 12: _t->func_ckb_MoveDir((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 13: _t->func_ckb_MoveFile((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 14: _t->func_ckb_newDir((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 15: _t->func_ckb_RenameDir((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 16: _t->func_ckb_RenameFile((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 17: _t->func_lne_textChanged((*reinterpret_cast< QString(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData WholeFileOpera_Setting::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject WholeFileOpera_Setting::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_WholeFileOpera_Setting,
      qt_meta_data_WholeFileOpera_Setting, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &WholeFileOpera_Setting::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *WholeFileOpera_Setting::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *WholeFileOpera_Setting::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_WholeFileOpera_Setting))
        return static_cast<void*>(const_cast< WholeFileOpera_Setting*>(this));
    return QWidget::qt_metacast(_clname);
}

int WholeFileOpera_Setting::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 18)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 18;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
