/****************************************************************************
** Meta object code from reading C++ file 'mainwindow.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../SkyEye/mainwindow.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainwindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Monitor[] = {

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
       9,    8,    8,    8, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_Monitor[] = {
    "Monitor\0\0haveData()\0"
};

void Monitor::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        Monitor *_t = static_cast<Monitor *>(_o);
        switch (_id) {
        case 0: _t->haveData(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData Monitor::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject Monitor::staticMetaObject = {
    { &QThread::staticMetaObject, qt_meta_stringdata_Monitor,
      qt_meta_data_Monitor, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Monitor::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Monitor::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Monitor::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Monitor))
        return static_cast<void*>(const_cast< Monitor*>(this));
    return QThread::qt_metacast(_clname);
}

int Monitor::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void Monitor::haveData()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}
static const uint qt_meta_data_MainWindow[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      27,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      12,   11,   11,   11, 0x05,

 // slots: signature, parameters, type, tag, flags
      24,   11,   11,   11, 0x08,
      35,   11,   11,   11, 0x08,
      57,   11,   11,   11, 0x08,
      78,   74,   11,   11, 0x08,
      99,   11,   11,   11, 0x08,
     114,   11,   11,   11, 0x08,
     133,   11,   11,   11, 0x08,
     154,   11,   11,   11, 0x08,
     175,   11,   11,   11, 0x08,
     193,   11,   11,   11, 0x08,
     214,   11,   11,   11, 0x08,
     234,   11,   11,   11, 0x08,
     255,   11,   11,   11, 0x08,
     278,   11,   11,   11, 0x08,
     298,   11,   11,   11, 0x08,
     320,   11,   11,   11, 0x08,
     337,   11,   11,   11, 0x08,
     352,   11,   11,   11, 0x08,
     366,   11,   11,   11, 0x08,
     386,   11,   11,   11, 0x08,
     408,   11,   11,   11, 0x08,
     426,   11,   11,   11, 0x08,
     448,   11,   11,   11, 0x08,
     480,  473,   11,   11, 0x08,
     529,   11,   11,   11, 0x08,
     540,   11,   11,   11, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_MainWindow[] = {
    "MainWindow\0\0tableShow()\0closesub()\0"
    "showCurrentUserInfo()\0showSqlManager()\0"
    "fdb\0getFromVerify(ForDB)\0showRegister()\0"
    "InitializeSkyEye()\0processSettingShow()\0"
    "NetheadSettingShow()\0FileSettingShow()\0"
    "clearUnusedProcess()\0insertTableWidget()\0"
    "sendNDISDriverMess()\0deleteNDISDriverMess()\0"
    "sendWFPDriverMess()\0deleteWFPDriverMess()\0"
    "getMonitorData()\0startMonitor()\0"
    "stopMonitor()\0sendSSDTProcessID()\0"
    "deleteSSDTProcessID()\0getUserPassword()\0"
    "addForbiddenProcess()\0deleteForbiddenProcess()\0"
    "reason\0iconIsActived(QSystemTrayIcon::ActivationReason)\0"
    "showTool()\0hideTool()\0"
};

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        MainWindow *_t = static_cast<MainWindow *>(_o);
        switch (_id) {
        case 0: _t->tableShow(); break;
        case 1: _t->closesub(); break;
        case 2: _t->showCurrentUserInfo(); break;
        case 3: _t->showSqlManager(); break;
        case 4: _t->getFromVerify((*reinterpret_cast< ForDB(*)>(_a[1]))); break;
        case 5: _t->showRegister(); break;
        case 6: _t->InitializeSkyEye(); break;
        case 7: _t->processSettingShow(); break;
        case 8: _t->NetheadSettingShow(); break;
        case 9: _t->FileSettingShow(); break;
        case 10: _t->clearUnusedProcess(); break;
        case 11: _t->insertTableWidget(); break;
        case 12: _t->sendNDISDriverMess(); break;
        case 13: _t->deleteNDISDriverMess(); break;
        case 14: _t->sendWFPDriverMess(); break;
        case 15: _t->deleteWFPDriverMess(); break;
        case 16: _t->getMonitorData(); break;
        case 17: _t->startMonitor(); break;
        case 18: _t->stopMonitor(); break;
        case 19: _t->sendSSDTProcessID(); break;
        case 20: _t->deleteSSDTProcessID(); break;
        case 21: _t->getUserPassword(); break;
        case 22: _t->addForbiddenProcess(); break;
        case 23: _t->deleteForbiddenProcess(); break;
        case 24: _t->iconIsActived((*reinterpret_cast< QSystemTrayIcon::ActivationReason(*)>(_a[1]))); break;
        case 25: _t->showTool(); break;
        case 26: _t->hideTool(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData MainWindow::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject MainWindow::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_MainWindow,
      qt_meta_data_MainWindow, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &MainWindow::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MainWindow))
        return static_cast<void*>(const_cast< MainWindow*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 27)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 27;
    }
    return _id;
}

// SIGNAL 0
void MainWindow::tableShow()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}
QT_END_MOC_NAMESPACE
