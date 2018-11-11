/****************************************************************************
** Meta object code from reading C++ file 'blacktablelist.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../SkyEye/blacktablelist.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'blacktablelist.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_BlackTableList[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      16,   15,   15,   15, 0x05,
      27,   15,   15,   15, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_BlackTableList[] = {
    "BlackTableList\0\0deleteDt()\0showDetail()\0"
};

void BlackTableList::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        BlackTableList *_t = static_cast<BlackTableList *>(_o);
        switch (_id) {
        case 0: _t->deleteDt(); break;
        case 1: _t->showDetail(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData BlackTableList::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject BlackTableList::staticMetaObject = {
    { &QTableWidget::staticMetaObject, qt_meta_stringdata_BlackTableList,
      qt_meta_data_BlackTableList, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &BlackTableList::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *BlackTableList::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *BlackTableList::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_BlackTableList))
        return static_cast<void*>(const_cast< BlackTableList*>(this));
    return QTableWidget::qt_metacast(_clname);
}

int BlackTableList::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QTableWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void BlackTableList::deleteDt()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void BlackTableList::showDetail()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}
QT_END_MOC_NAMESPACE
