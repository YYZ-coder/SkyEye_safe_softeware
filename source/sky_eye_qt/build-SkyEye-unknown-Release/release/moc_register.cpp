/****************************************************************************
** Meta object code from reading C++ file 'register.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../SkyEye/register.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'register.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Register[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      12,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      10,    9,    9,    9, 0x0a,
      26,    9,    9,    9, 0x0a,
      45,    9,    9,    9, 0x0a,
      67,    9,    9,    9, 0x0a,
      83,    9,    9,    9, 0x0a,
      98,    9,    9,    9, 0x0a,
     112,    9,    9,    9, 0x0a,
     126,    9,    9,    9, 0x0a,
     142,    9,    9,    9, 0x0a,
     155,    9,    9,    9, 0x0a,
     173,    9,    9,    9, 0x0a,
     191,    9,    9,    9, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_Register[] = {
    "Register\0\0processNet(int)\0processCreate(int)\0"
    "netheadForbidden(int)\0fileModify(int)\0"
    "fileWrite(int)\0fileRead(int)\0fileCopy(int)\0"
    "fileExcute(int)\0fileAll(int)\0"
    "showRightgather()\0GenerateAccount()\0"
    "createAccount()\0"
};

void Register::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        Register *_t = static_cast<Register *>(_o);
        switch (_id) {
        case 0: _t->processNet((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->processCreate((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->netheadForbidden((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->fileModify((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->fileWrite((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: _t->fileRead((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 6: _t->fileCopy((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 7: _t->fileExcute((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 8: _t->fileAll((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 9: _t->showRightgather(); break;
        case 10: _t->GenerateAccount(); break;
        case 11: _t->createAccount(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData Register::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject Register::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_Register,
      qt_meta_data_Register, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Register::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Register::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Register::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Register))
        return static_cast<void*>(const_cast< Register*>(this));
    return QDialog::qt_metacast(_clname);
}

int Register::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 12)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 12;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
