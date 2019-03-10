#include "stdafx.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QAction>
#include <QSettings>
#include <QVBoxLayout>
#include <QDockWidget>
#include <QLabel>
#include "main.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qscreen.h"
#include "qkeyboardview.h"
#include "qconsoleview.h"
#include "qdebugview.h"
#include "qdisasmview.h"
#include "qmemoryview.h"
#include "qscripting.h"
#include "Emulator.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setWindowTitle(tr("UKNC Back to Life"));

    // Assign signals
    QObject::connect(ui->actionSaveStateImage, SIGNAL(triggered()), this, SLOT(saveStateImage()));
    QObject::connect(ui->actionLoadStateImage, SIGNAL(triggered()), this, SLOT(loadStateImage()));
    QObject::connect(ui->actionFileScreenshot, SIGNAL(triggered()), this, SLOT(saveScreenshot()));
    QObject::connect(ui->actionScriptRun, SIGNAL(triggered()), this, SLOT(scriptRun()));
    QObject::connect(ui->actionFileExit, SIGNAL(triggered()), this, SLOT(close()));
    QObject::connect(ui->actionEmulatorRun, SIGNAL(triggered()), this, SLOT(emulatorRun()));
    QObject::connect(ui->actionEmulatorReset, SIGNAL(triggered()), this, SLOT(emulatorReset()));
    QObject::connect(ui->actionDrivesFloppy0, SIGNAL(triggered()), this, SLOT(emulatorFloppy0()));
    QObject::connect(ui->actionDrivesFloppy1, SIGNAL(triggered()), this, SLOT(emulatorFloppy1()));
    QObject::connect(ui->actionDrivesFloppy2, SIGNAL(triggered()), this, SLOT(emulatorFloppy2()));
    QObject::connect(ui->actionDrivesFloppy3, SIGNAL(triggered()), this, SLOT(emulatorFloppy3()));
    QObject::connect(ui->actionDrivesCartridge1, SIGNAL(triggered()), this, SLOT(emulatorCartridge1()));
    QObject::connect(ui->actionDrivesHard1, SIGNAL(triggered()), this, SLOT(emulatorHardDrive1()));
    QObject::connect(ui->actionDrivesCartridge2, SIGNAL(triggered()), this, SLOT(emulatorCartridge2()));
    QObject::connect(ui->actionDrivesHard2, SIGNAL(triggered()), this, SLOT(emulatorHardDrive2()));
    QObject::connect(ui->actionDebugConsoleView, SIGNAL(triggered()), this, SLOT(debugConsoleView()));
    QObject::connect(ui->actionDebugDebugView, SIGNAL(triggered()), this, SLOT(debugDebugView()));
    QObject::connect(ui->actionDebugDisasmView, SIGNAL(triggered()), this, SLOT(debugDisasmView()));
    QObject::connect(ui->actionDebugMemoryView, SIGNAL(triggered()), this, SLOT(debugMemoryView()));
    QObject::connect(ui->actionDebugStepInto, SIGNAL(triggered()), this, SLOT(debugStepInto()));
    QObject::connect(ui->actionDebugStepOver, SIGNAL(triggered()), this, SLOT(debugStepOver()));
    QObject::connect(ui->actionHelpAbout, SIGNAL(triggered()), this, SLOT(helpAbout()));
    QObject::connect(ui->actionViewKeyboard, SIGNAL(triggered()), this, SLOT(viewKeyboard()));
    QObject::connect(ui->actionViewRgbScreen, SIGNAL(triggered()), this, SLOT(viewRgbScreen()));
    QObject::connect(ui->actionViewGrayscaleScreen, SIGNAL(triggered()), this, SLOT(viewGrayscaleScreen()));
    QObject::connect(ui->actionViewSizeRegular, SIGNAL(triggered()), this, SLOT(viewSizeRegular()));
    QObject::connect(ui->actionViewSizeDouble, SIGNAL(triggered()), this, SLOT(viewSizeDouble()));
    QObject::connect(ui->actionViewSizeUpscaled, SIGNAL(triggered()), this, SLOT(viewSizeUpscaled()));
    QObject::connect(ui->actionViewSizeUpscaled3, SIGNAL(triggered()), this, SLOT(viewSizeUpscaled3()));
    QObject::connect(ui->actionViewSizeUpscaled4, SIGNAL(triggered()), this, SLOT(viewSizeUpscaled4()));
    QObject::connect(ui->actionSoundEnabled, SIGNAL(triggered()), this, SLOT(soundEnabled()));

    // Screen and keyboard
    m_screen = new QEmulatorScreen();
    m_keyboard = new QKeyboardView();
    m_console = new QConsoleView();
    m_debug = new QDebugView(this);
    m_disasm = new QDisasmView();
    m_memory = new QMemoryView();

    QVBoxLayout *vboxlayout = new QVBoxLayout;
    vboxlayout->setMargin(4);
    vboxlayout->setSpacing(4);
    vboxlayout->addWidget(m_screen);
    vboxlayout->addWidget(m_keyboard);
    ui->centralWidget->setLayout(vboxlayout);
    ui->centralWidget->setMaximumHeight(m_screen->maximumHeight() + m_keyboard->maximumHeight());
    int maxwid = m_screen->maximumWidth() > m_keyboard->maximumWidth() ? m_screen->maximumWidth() : m_keyboard->maximumWidth();
    ui->centralWidget->setMaximumWidth(maxwid);

    m_dockDebug = new QDockWidget(tr("Processor"));
    m_dockDebug->setObjectName("dockDebug");
    m_dockDebug->setWidget(m_debug);
    m_dockDisasm = new QDockWidget(tr("Disassemble"));
    m_dockDisasm->setObjectName("dockDisasm");
    m_dockDisasm->setWidget(m_disasm);
    m_dockMemory = new QDockWidget(tr("Memory"));
    m_dockMemory->setObjectName("dockMemory");
    m_dockMemory->setWidget(m_memory);
    m_dockConsole = new QDockWidget(tr("Debug Console"));
    m_dockConsole->setObjectName("dockConsole");
    m_dockConsole->setWidget(m_console);

    this->setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);
    this->setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
    this->addDockWidget(Qt::RightDockWidgetArea, m_dockDebug, Qt::Vertical);
    this->addDockWidget(Qt::RightDockWidgetArea, m_dockDisasm, Qt::Vertical);
    this->addDockWidget(Qt::RightDockWidgetArea, m_dockMemory, Qt::Vertical);
    this->addDockWidget(Qt::BottomDockWidgetArea, m_dockConsole);

    m_statusLabelInfo = new QLabel(this);
    m_statusLabelFrames = new QLabel(this);
    m_statusLabelUptime = new QLabel(this);
    statusBar()->addWidget(m_statusLabelInfo, 600);
    statusBar()->addPermanentWidget(m_statusLabelFrames, 150);
    statusBar()->addPermanentWidget(m_statusLabelUptime, 150);

    this->setFocusProxy(m_screen);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_screen;
    delete m_keyboard;
    delete m_console;
    delete m_debug;
    delete m_disasm;
    delete m_memory;
    delete m_dockConsole;
    delete m_dockDebug;
    delete m_dockDisasm;
    delete m_dockMemory;
    delete m_statusLabelInfo;
    delete m_statusLabelFrames;
    delete m_statusLabelUptime;
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type())
    {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void MainWindow::closeEvent(QCloseEvent *)
{
    Global_getSettings()->setValue("MainWindow/ScreenViewMode", m_screen->mode());
    Global_getSettings()->setValue("MainWindow/ScreenSizeMode", m_screen->sizeMode());

    Global_getSettings()->setValue("MainWindow/Geometry", saveGeometry());
    Global_getSettings()->setValue("MainWindow/WindowState", saveState());

    Global_getSettings()->setValue("MainWindow/ConsoleView", m_dockConsole->isVisible());
    Global_getSettings()->setValue("MainWindow/DebugView", m_dockDebug->isVisible());
    Global_getSettings()->setValue("MainWindow/DisasmView", m_dockDisasm->isVisible());
    Global_getSettings()->setValue("MainWindow/MemoryView", m_dockMemory->isVisible());
}

void MainWindow::restoreSettings()
{
    ScreenViewMode scrViewMode = (ScreenViewMode)Global_getSettings()->value("MainWindow/ScreenViewMode").toInt();
    if (scrViewMode == 0) scrViewMode = RGBScreen;
    m_screen->setMode(scrViewMode);
    ScreenSizeMode scrSizeMode = (ScreenSizeMode)Global_getSettings()->value("MainWindow/ScreenSizeMode").toInt();
    if (scrSizeMode == 0) scrSizeMode = RegularScreen;
    m_screen->setSizeMode(scrSizeMode);

    //Update centralWidget size
    ui->centralWidget->setMaximumHeight(m_screen->maximumHeight() + m_keyboard->maximumHeight());
    ui->centralWidget->setMaximumWidth(m_screen->maximumWidth());

    //NOTE: Restore from maximized state fails, see https://bugreports.qt-project.org/browse/QTBUG-15080
    restoreGeometry(Global_getSettings()->value("MainWindow/Geometry").toByteArray());
    restoreState(Global_getSettings()->value("MainWindow/WindowState").toByteArray());

    m_dockConsole->setVisible(Global_getSettings()->value("MainWindow/ConsoleView", false).toBool());
    m_dockDebug->setVisible(Global_getSettings()->value("MainWindow/DebugView", false).toBool());
    m_dockDisasm->setVisible(Global_getSettings()->value("MainWindow/DisasmView", false).toBool());
    m_dockMemory->setVisible(Global_getSettings()->value("MainWindow/MemoryView", false).toBool());

    ui->actionSoundEnabled->setChecked(Settings_GetSound());
    m_debug->updateWindowText();
    m_disasm->updateWindowText();
    m_memory->updateWindowText();
}

void MainWindow::UpdateMenu()
{
    ui->actionEmulatorRun->setChecked(g_okEmulatorRunning);
    ui->actionViewRgbScreen->setChecked(m_screen->mode() == RGBScreen);
    ui->actionViewGrayscaleScreen->setChecked(m_screen->mode() == GrayScreen);
    ui->actionViewSizeRegular->setChecked(m_screen->sizeMode() == RegularScreen);
    ui->actionViewSizeUpscaled->setChecked(m_screen->sizeMode() == UpscaledScreen);
    ui->actionViewSizeDouble->setChecked(m_screen->sizeMode() == DoubleScreen);
    ui->actionViewSizeUpscaled3->setChecked(m_screen->sizeMode() == UpscaledScreen3);
    ui->actionViewSizeUpscaled4->setChecked(m_screen->sizeMode() == UpscaledScreen4);

    ui->actionViewKeyboard->setChecked(m_keyboard->isVisible());

    ui->actionDrivesFloppy0->setIcon(QIcon(
            g_pBoard->IsFloppyImageAttached(0) ? ":/images/iconFloppy.png" : ":/images/iconFloppySlot.png" ));
    ui->actionDrivesFloppy1->setIcon(QIcon(
            g_pBoard->IsFloppyImageAttached(1) ? ":/images/iconFloppy.png" : ":/images/iconFloppySlot.png" ));
    ui->actionDrivesFloppy2->setIcon(QIcon(
            g_pBoard->IsFloppyImageAttached(2) ? ":/images/iconFloppy.png" : ":/images/iconFloppySlot.png" ));
    ui->actionDrivesFloppy3->setIcon(QIcon(
            g_pBoard->IsFloppyImageAttached(3) ? ":/images/iconFloppy.png" : ":/images/iconFloppySlot.png" ));

    ui->actionDrivesCartridge1->setIcon(QIcon(
            g_pBoard->IsROMCartridgeLoaded(1) ? ":/images/iconCartridge.png" : ":/images/iconCartridgeSlot.png" ));
    ui->actionDrivesCartridge2->setIcon(QIcon(
            g_pBoard->IsROMCartridgeLoaded(2) ? ":/images/iconCartridge.png" : ":/images/iconCartridgeSlot.png" ));

    ui->actionDrivesHard1->setIcon(QIcon(
            g_pBoard->IsHardImageAttached(1) ? ":/images/iconHdd.png" : ":/images/iconHddSlot.png" ));
    ui->actionDrivesHard2->setIcon(QIcon(
            g_pBoard->IsHardImageAttached(2) ? ":/images/iconHdd.png" : ":/images/iconHddSlot.png" ));

    ui->actionDebugConsoleView->setChecked(m_console->isVisible());
    ui->actionDebugDebugView->setChecked(m_dockDebug->isVisible());
    ui->actionDebugDisasmView->setChecked(m_dockDisasm->isVisible());
    ui->actionDebugMemoryView->setChecked(m_dockMemory->isVisible());
}

void MainWindow::UpdateAllViews()
{
    Emulator_OnUpdate();

    if (m_debug != nullptr)
        m_debug->updateData();
    if (m_disasm != nullptr)
        m_disasm->updateData();
    if (m_memory != nullptr)
        m_memory->updateData();
    if (m_console != nullptr)
        m_console->updatePrompt();

    m_screen->repaint();
    if (m_debug != nullptr)
        m_debug->repaint();
    if (m_disasm != nullptr)
        m_disasm->repaint();
    if (m_memory != nullptr)
        m_memory->repaint();

    UpdateMenu();
}

void MainWindow::setCurrentProc(bool okProc)
{
    if (m_debug != nullptr)
        m_debug->setCurrentProc(okProc);
    if (m_disasm != nullptr)
        m_disasm->setCurrentProc(okProc);
    if (m_console != nullptr)
        m_console->setCurrentProc(okProc);
}

void MainWindow::showUptime(int uptimeMillisec)
{
    int seconds = (int) (uptimeMillisec % 60);
    int minutes = (int) (uptimeMillisec / 60 % 60);
    int hours   = (int) (uptimeMillisec / 3600 % 60);

    char buffer[12];
    _snprintf(buffer, 20, "%02d:%02d:%02d", hours, minutes, seconds);
    m_statusLabelUptime->setText(tr("Uptime: %1").arg(buffer));
}
void MainWindow::showFps(double framesPerSecond)
{
    if (framesPerSecond <= 0)
    {
        m_statusLabelFrames->setText("");
    }
    else
    {
        double speed = framesPerSecond / 25.0 * 100.0;
        char buffer[16];
        _snprintf(buffer, 16, "%03.f%%", speed);
        m_statusLabelFrames->setText(buffer);
    }
}

void MainWindow::saveStateImage()
{
    QFileDialog dlg;
    dlg.setAcceptMode(QFileDialog::AcceptSave);
    dlg.setNameFilter(tr("UKNC state images (*.uknc)"));
    if (dlg.exec() == QDialog::Rejected)
        return;

    QString strFileName = dlg.selectedFiles().at(0);

    saveStateImage(strFileName);
}
void MainWindow::saveStateImage(const QString& strFileName)
{
    LPCTSTR sFileName = qPrintable(strFileName);
    Emulator_SaveImage(sFileName);
}
void MainWindow::loadStateImage()
{
    QFileDialog dlg;
    dlg.setNameFilter(tr("UKNC state images (*.uknc)"));
    if (dlg.exec() == QDialog::Rejected)
        return;

    QString strFileName = dlg.selectedFiles().at(0);

    loadStateImage(strFileName);
}
void MainWindow::loadStateImage(const QString& strFileName)
{
    LPCTSTR sFileName = qPrintable(strFileName);
    Emulator_LoadImage(sFileName);

    UpdateAllViews();
}

void MainWindow::saveScreenshot()
{
    QFileDialog dlg;
    dlg.setAcceptMode(QFileDialog::AcceptSave);
    dlg.setNameFilter(tr("PNG images (*.png)"));
    if (dlg.exec() == QDialog::Rejected)
        return;

    QString strFileName = dlg.selectedFiles().at(0);

    saveScreenshot(strFileName);
}
void MainWindow::saveScreenshot(const QString& strFileName)
{
    m_screen->saveScreenshot(strFileName);
}

void MainWindow::helpAbout()
{
    QMessageBox::about(this, tr("About"), tr(
            "UKNCBTL Qt Version 1.0\n"
            "Copyright (C) 2007-2019\n\n"
            "https://github.com/nzeemin/ukncbtl-qt\n\n"
            "Authors:\r\nNikita Zimin\nFelix Lazarev\nAlexey Kisly\n\n"
            "Special thanks to:\nArseny Gordin\n\n"
            "Build date:\t%1 %2\n"
            "Qt version:\t%3")
            .arg(__DATE__).arg(__TIME__).arg(QT_VERSION_STR));
}

void MainWindow::viewKeyboard()
{
    m_keyboard->setVisible(!m_keyboard->isVisible());
    UpdateMenu();
}

void MainWindow::viewRgbScreen()
{
    m_screen->setMode(RGBScreen);
    UpdateMenu();
}
void MainWindow::viewGrayscaleScreen()
{
    m_screen->setMode(GrayScreen);
    UpdateMenu();
}

void MainWindow::viewSizeRegular()
{
    m_screen->setSizeMode(RegularScreen);
    UpdateMenu();

    //Update centralWidget size
    ui->centralWidget->setMaximumHeight(m_screen->maximumHeight() + m_keyboard->maximumHeight());
    ui->centralWidget->setMaximumWidth(m_screen->maximumWidth());
}
void MainWindow::viewSizeUpscaled()
{
    m_screen->setSizeMode(UpscaledScreen);
    UpdateMenu();

    //Update centralWidget size
    ui->centralWidget->setMaximumHeight(m_screen->maximumHeight() + m_keyboard->maximumHeight());
    ui->centralWidget->setMaximumWidth(m_screen->maximumWidth());
}
void MainWindow::viewSizeDouble()
{
    m_screen->setSizeMode(DoubleScreen);
    UpdateMenu();

    //Update centralWidget size
    ui->centralWidget->setMaximumHeight(m_screen->maximumHeight() + m_keyboard->maximumHeight());
    ui->centralWidget->setMaximumWidth(m_screen->maximumWidth());
}
void MainWindow::viewSizeUpscaled3()
{
    m_screen->setSizeMode(UpscaledScreen3);
    UpdateMenu();

    //Update centralWidget size
    ui->centralWidget->setMaximumHeight(m_screen->maximumHeight() + m_keyboard->maximumHeight());
    ui->centralWidget->setMaximumWidth(m_screen->maximumWidth());
}
void MainWindow::viewSizeUpscaled4()
{
    m_screen->setSizeMode(UpscaledScreen4);
    UpdateMenu();

    //Update centralWidget size
    ui->centralWidget->setMaximumHeight(m_screen->maximumHeight() + m_keyboard->maximumHeight());
    ui->centralWidget->setMaximumWidth(m_screen->maximumWidth());
}

void MainWindow::emulatorFrame()
{
    if (!g_okEmulatorRunning)
        return;
    if (!isActiveWindow())
        return;

    if (Emulator_IsBreakpoint())
        Emulator_Stop();
    else if (Emulator_SystemFrame())
    {
        m_screen->repaint();
    }
}

void MainWindow::emulatorRun()
{
    if (g_okEmulatorRunning)
    {
        this->setWindowTitle(tr("UKNC Back to Life"));
        Emulator_Stop();
    }
    else
    {
        this->setWindowTitle(tr("UKNC Back to Life [run]"));
        Emulator_Start();
    }
}

void MainWindow::emulatorReset()
{
    Emulator_Reset();

    m_screen->repaint();
}

void MainWindow::soundEnabled()
{
    bool sound = ui->actionSoundEnabled->isChecked();
    Emulator_SetSound(sound ? true : false);
    Settings_SetSound(sound);
}

void MainWindow::emulatorCartridge1() { emulatorCartridge(1); }
void MainWindow::emulatorCartridge2() { emulatorCartridge(2); }
void MainWindow::emulatorCartridge(int slot)
{
    if (g_pBoard->IsROMCartridgeLoaded(slot))
    {
        detachCartridge(slot);
    }
    else
    {
        QFileDialog dlg;
        dlg.setNameFilter(tr("UKNC ROM cartridge images (*.bin)"));
        if (dlg.exec() == QDialog::Rejected)
            return;

        QString strFileName = dlg.selectedFiles().at(0);
        if (!attachCartridge(slot, strFileName))
            return;
    }
}
bool MainWindow::attachCartridge(int slot, const QString & strFileName)
{
    QFileInfo fi(strFileName);
    QString strFullName(fi.canonicalFilePath());  // Get absolute file name

    LPCTSTR sFileName = qPrintable(strFullName);
    Emulator_LoadROMCartridge(slot, sFileName);
    //TODO: Check result

    Settings_SetCartridgeFilePath(slot, strFullName);

    UpdateMenu();

    return true;
}
void MainWindow::detachCartridge(int slot)
{
    g_pBoard->UnloadROMCartridge(slot);

    Settings_SetCartridgeFilePath(slot, nullptr);

    UpdateMenu();
}

void MainWindow::emulatorFloppy0() { emulatorFloppy(0); }
void MainWindow::emulatorFloppy1() { emulatorFloppy(1); }
void MainWindow::emulatorFloppy2() { emulatorFloppy(2); }
void MainWindow::emulatorFloppy3() { emulatorFloppy(3); }
void MainWindow::emulatorFloppy(int slot)
{
    if (g_pBoard->IsFloppyImageAttached(slot))
    {
        detachFloppy(slot);
    }
    else
    {
        QFileDialog dlg;
        dlg.setNameFilter(tr("UKNC floppy images (*.dsk *.rtd)"));
        if (dlg.exec() == QDialog::Rejected)
            return;

        QString strFileName = dlg.selectedFiles().at(0);

        if (! attachFloppy(slot, strFileName))
        {
            AlertWarning(tr("Failed to attach floppy image."));
            return;
        }
    }
}
bool MainWindow::attachFloppy(int slot, const QString & strFileName)
{
    QFileInfo fi(strFileName);
    QString strFullName(fi.canonicalFilePath());  // Get absolute file name

    LPCTSTR sFileName = qPrintable(strFullName);
    if (! g_pBoard->AttachFloppyImage(slot, sFileName))
        return false;

    Settings_SetFloppyFilePath(slot, strFullName);

    UpdateMenu();

    return true;
}
void MainWindow::detachFloppy(int slot)
{
    g_pBoard->DetachFloppyImage(slot);

    Settings_SetFloppyFilePath(slot, nullptr);

    UpdateMenu();
}

void MainWindow::emulatorHardDrive1() { emulatorHardDrive(1); }
void MainWindow::emulatorHardDrive2() { emulatorHardDrive(2); }
void MainWindow::emulatorHardDrive(int slot)
{
    if (g_pBoard->IsHardImageAttached(slot))
    {
        detachHardDrive(slot);
    }
    else
    {
        // Check if cartridge (HDD ROM image) already selected
        bool okCartLoaded = g_pBoard->IsROMCartridgeLoaded(slot);
        if (!okCartLoaded)
        {
            AlertWarning(tr("Please select HDD ROM image as cartridge first."));
            return;
        }

        // Select HDD disk image
        QFileDialog dlg;
        dlg.setNameFilter(tr("UKNC HDD images (*.img)"));
        if (dlg.exec() == QDialog::Rejected)
            return;

        QString strFileName = dlg.selectedFiles().at(0);
        if (! attachHardDrive(slot, strFileName))
        {
            AlertWarning(tr("Failed to attach hard drive image."));
            return;
        }
    }
}
bool MainWindow::attachHardDrive(int slot, const QString & strFileName)
{
    if (!g_pBoard->IsROMCartridgeLoaded(slot))
        return false;

    QFileInfo fi(strFileName);
    QString strFullName(fi.canonicalFilePath());  // Get absolute file name

    LPCTSTR sFileName = qPrintable(strFullName);
    if (!g_pBoard->AttachHardImage(slot, sFileName))
        return false;

    Settings_SetHardFilePath(slot, strFullName);

    UpdateMenu();

    return true;
}
void MainWindow::detachHardDrive(int slot)
{
    g_pBoard->DetachHardImage(slot);
    Settings_SetHardFilePath(slot, nullptr);
}

void MainWindow::debugConsoleView()
{
    bool okShow = !m_dockConsole->isVisible();
    m_dockConsole->setVisible(okShow);
    m_dockDebug->setVisible(okShow);
    m_dockDisasm->setVisible(okShow);
    m_dockMemory->setVisible(okShow);

    if (!okShow)
    {
        this->adjustSize();
    }

    UpdateMenu();
}
void MainWindow::debugDebugView()
{
    m_dockDebug->setVisible(!m_dockDebug->isVisible());
    UpdateMenu();
}
void MainWindow::debugDisasmView()
{
    m_dockDisasm->setVisible(!m_dockDisasm->isVisible());
    UpdateMenu();
}
void MainWindow::debugMemoryView()
{
    m_dockMemory->setVisible(!m_dockMemory->isVisible());
    UpdateMenu();
}

void MainWindow::debugStepInto()
{
    if (!g_okEmulatorRunning)
        m_console->execConsoleCommand("s");
}
void MainWindow::debugStepOver()
{
    if (!g_okEmulatorRunning)
        m_console->execConsoleCommand("so");
}

void MainWindow::scriptRun()
{
    if (g_okEmulatorRunning)
        emulatorRun();  // Stop the emulator

    QFileDialog dlg;
    dlg.setAcceptMode(QFileDialog::AcceptOpen);
    dlg.setNameFilter(tr("Script files (*.js)"));
    if (dlg.exec() == QDialog::Rejected)
        return;

    QString strFileName = dlg.selectedFiles().at(0);
    QFile file(strFileName);
    file.open(QIODevice::ReadOnly);
    QString strScript = file.readAll();

    QScriptWindow window(this);
    window.runScript(strScript);
}

void MainWindow::consolePrint(const QString &message)
{
    m_console->printLine(message);
}
