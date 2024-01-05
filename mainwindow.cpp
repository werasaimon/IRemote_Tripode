#include "mainwindow.h"
#include <QDebug>
#include "ui_mainwindow.h"

#include <algorithm>


namespace
{
  static char buttonState = 0;
  static int a, b, c, d, e, f;
} // namespace

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , joystick(NULL)
{
    ui->setupUi(this);

    this->setWindowTitle("Davinchi-Hexapod-UI");

    mControlMode = "Joystik-Controls";

    // create a QUDP socket
    mSocketUDP = new QUdpSocket(this);
    m_Port = 8888;

    connect(mSocketUDP, SIGNAL(readyRead()), this, SLOT(readPendingDatagrams()));

    mSerialPort = new QSerialPort(this);
    connect(mSerialPort, SIGNAL(readyRead()), this, SLOT(handleReadSerialData()));

    mTimer = new QTimer(this);
    connect(mTimer, SIGNAL(timeout()), this, SLOT(update_timer()));

    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
    foreach (const QSerialPortInfo &portInfo, ports) {
        ui->comboBox_SerialPorts->addItem(portInfo.portName());
    }

    //----------------------------------------------------//

    ui->plainTextEdit_Output->setReadOnly(true);

    mTimeShot = "all";
    ui->comboBox_TimeShot->setCurrentText(mTimeShot);

    mIsShot = false;
    mISControl = false;
    mISFuseBlown = true;

    angle_yaw = 0;
    angle_pitch = 0;
    angle_roll = 0;

    invert = 1;

    mJoystick.Initilization(0);
}

MainWindow::~MainWindow()
{
    if (mSerialPort) {
        delete mSerialPort;
        mSerialPort = nullptr;
    }

    if (joystick) {
        JoystickClose(joystick);
        //delete joystick;
        joystick = nullptr;
    }

    mJoystick.Close();

    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    event->accept();
    if (mSerialPort->isOpen()) {
        mSerialPort->close();
    }

    SDL_Quit();
}

SDL_Joystick *MainWindow::JoystickInit(int index_joystick)
{
    // Инициализация SDL для использования джойстика
    SDL_Init(SDL_INIT_JOYSTICK);
    // Включаем
    SDL_JoystickEventState(SDL_ENABLE);

    // If there are no joysticks connected, quit the program
    if (SDL_NumJoysticks() <= 0) {
        printf("There are no joysticks connected. Quitting now...\n");
        SDL_Quit();
    }

    // Open the joystick for reading and store its handle in the joy variable
    auto joystick = SDL_JoystickOpen(index_joystick);

    //----------------------------------------------------//

    if (joystick != NULL) {
        // Get information about the joystick
        name = SDL_JoystickName(joystick);
        num_axes = SDL_JoystickNumAxes(joystick);
        num_buttons = SDL_JoystickNumButtons(joystick);
        num_hats = SDL_JoystickNumHats(joystick);

        printf("Now reading from joystick '%s' with:\n"
               "%d axes\n"
               "%d buttons\n"
               "%d hats\n\n",
               name,
               num_axes,
               num_buttons,
               num_hats);
    }

    return joystick;
}

void MainWindow::JoystickClose(SDL_Joystick *_joystick)
{
    SDL_JoystickClose(_joystick);
}

void MainWindow::handleReadSerialData()
{
    QByteArray data = mSerialPort->readAll();
    // Process the received data
    qDebug() << "UART: " << data;
    ui->plainTextEdit_Output->appendPlainText(data.toStdString().c_str());
}

void MainWindow::handleConnectAndSendCommand()
{
    if (mSerialPort) {
        if (ui->comboBox_SerialPorts->currentText().isEmpty())
            return;

        mSerialPort->setPortName(ui->comboBox_SerialPorts->currentText());

        if (mSerialPort->isOpen())
            mSerialPort->close();

        // mSerialPort->setBaudRate(QSerialPort::Baud115200);
        // mSerialPort->setDataBits(QSerialPort::Data8);
        // mSerialPort->setParity(QSerialPort::NoParity);
        //mSerialPort->setStopBits(QSerialPort::OneStop);
        // mSerialPort->setFlowControl(QSerialPort::NoFlowControl);

        mSerialPort->setBaudRate(QSerialPort::Baud115200);
        mSerialPort->setDataBits(QSerialPort::Data8);
        mSerialPort->setParity(QSerialPort::NoParity);
        mSerialPort->setStopBits(QSerialPort::OneStop);
        mSerialPort->setFlowControl(
            QSerialPort::HardwareControl); // Используйте аппаратное управление потоком

        if (mSerialPort->open(QIODevice::ReadWrite)) {
            //           mSerialPort->setTextModeEnabled(true);
            //           mSerialPort->write("Hello\r\n");
            qDebug() << "Port open at " << mSerialPort->portName();
            ui->plainTextEdit_Output->appendPlainText(QString("Port open at : ")
                                                      + mSerialPort->portName());

        } else {
            qDebug() << "Failed to open tty port " << mSerialPort->portName();
            ui->plainTextEdit_Output->appendPlainText(QString("Failed to open tty port : ")
                                                      + mSerialPort->portName());
        }

        //       QByteArray data;
        //       QDataStream stream(&data, QIODevice::WriteOnly);
        //       stream << 0; // Write the number to the byte array
        //       mSerialPort->write(data);
    }
}

void MainWindow::sendCommandToSerialPortAndUDP()
{
    int16_t buttonState = 0;
    int a, b, c, d, e, f;
    buttonState = false;

    if (mControlMode == "InvidualControls-Velocity")
    {
        a = ui->horizontalSlider_A->value();
        b = ui->horizontalSlider_B->value();
        c = ui->horizontalSlider_C->value();
        d = ui->horizontalSlider_D->value();
        e = ui->horizontalSlider_E->value();
        f = ui->horizontalSlider_F->value();

        buttonState |= (true << 0);  // Первая кнопка: бит 0 (младший бит)
        buttonState |= (false << 1); // Вторая кнопка: бит 1
        buttonState |= (false << 2); // Третья кнопка: бит 2
        buttonState |= (false << 3); // Четвертая кнопка: бит 3
        buttonState |= (false << 4); // Режим управления каждим акутатором отдельно . бит 5
        buttonState |= (mIsShot << 5); // Режим управления каждим акутатором отдельно по скорости
        buttonState |= (false << 6);
        buttonState |= (mISFuseBlown << 7);

    }
    else if (mControlMode == "InvidualControls-Position")
    {
        a = ui->horizontalSlider_A->value();
        b = ui->horizontalSlider_B->value();
        c = ui->horizontalSlider_C->value();
        d = ui->horizontalSlider_D->value();
        e = ui->horizontalSlider_E->value();
        f = ui->horizontalSlider_F->value();

        buttonState |= (false << 0); // Первая кнопка: бит 0 (младший бит)
        buttonState |= (true << 1);  // Вторая кнопка: бит 1
        buttonState |= (false << 2); // Третья кнопка: бит 2
        buttonState |= (false << 3); // Четвертая кнопка: бит 3
        buttonState |= (false << 4); // Режим управления каждим акутатором отдельно . бит 5
        buttonState |= (mIsShot << 5); // Режим управления каждим акутатором отдельно по скорости
        buttonState |= (false << 6);
        buttonState |= (mISFuseBlown << 7);

    }
    else if (mControlMode == "Speed-Controls")
    {
        a = ui->horizontalSlider_A->value();
        b = ui->horizontalSlider_B->value();
        c = ui->horizontalSlider_C->value();
        d = ui->horizontalSlider_D->value();
        e = ui->horizontalSlider_E->value();
        f = ui->horizontalSlider_F->value();

        buttonState |= (false << 0); // Первая кнопка: бит 0 (младший бит)
        buttonState |= (false << 1); // Вторая кнопка: бит 1
        buttonState |= (true << 2);  // Третья кнопка: бит 2
        buttonState |= (false << 3); // Четвертая кнопка: бит 3
        buttonState |= (false << 4); // Режим управления каждим акутатором отдельно . бит 5
        buttonState |= (mIsShot << 5); // Режим управления каждим акутатором отдельно по скорости
        buttonState |= (false << 6);
        buttonState |= (mISFuseBlown << 7);

        bool button0 = (buttonState >> 0) & 1;
        bool button1 = (buttonState >> 1) & 1;
        bool button2 = (buttonState >> 2) & 1;

        qDebug() << "ba" << button0 << "bb" << button1 << "bc" << button2;
    }
    else if (mControlMode == "Joystik-Controls")
    {
        if (SDL_QuitRequested())
        {
            mTimer->stop();
        }

        if (SDL_IsGameController(0))
        {
            float size_diff = 4096;
            float size_ang = 4096;
            //          a = (SDL_JoystickGetAxis(joystick, 0) / 32768.f) * size_diff;
            //          b =-(SDL_JoystickGetAxis(joystick, 1) / 32768.f) * size_diff;
            //          c = (SDL_JoystickGetAxis(joystick, 3) / 32768.f) * size_ang;
            //          d = (SDL_JoystickGetAxis(joystick, 4) / 32768.f) * size_ang;
            //          e =  0;//SDL_JoystickGetButton(joystick, 6);
            //          f =  0;//SDL_JoystickGetButton(joystick, 7);

            a = (SDL_JoystickGetAxis(joystick, 0) / 32768.f) * size_diff * invert;
            b = (SDL_JoystickGetAxis(joystick, 1) / 32768.f) * size_diff * invert;
            c = (SDL_JoystickGetAxis(joystick, 3) / 32768.f) * size_ang;
            d = (SDL_JoystickGetAxis(joystick, 4) / 32768.f) * size_ang;

            //e = ((SDL_JoystickGetAxis(joystick, 2) / 32768.f) * size_ang) + 4096;
            f = 0; //SDL_JoystickGetButton(joystick, 7);

            a = (std::abs(a) < 50) ? 0 : a;
            b = (std::abs(b) < 50) ? 0 : b;
            c = (std::abs(c) < 50) ? 0 : c;
            d = (std::abs(d) < 50) ? 0 : d;
            e = (std::abs(e) < 50) ? 0 : e;
            f = (std::abs(f) < 50) ? 0 : f;

            qDebug() << a << b << c << d << e << SDL_JoystickGetButton(joystick, 4);


            std::swap(b,d);
            //--------------------------------------------//

            bool button_joystick_0 = SDL_JoystickGetButton(joystick, 4);
            bool button_joystick_1 = SDL_JoystickGetButton(joystick, 5);
            bool button_joystick_2 = SDL_JoystickGetButton(joystick, 6);
            bool button_joystick_3 = SDL_JoystickGetButton(joystick, 7);

            buttonState = false;
            buttonState |= (false << 0); // Первая кнопка: бит 0 (младший бит)
            buttonState |= (false << 1); // Вторая кнопка: бит 1
            buttonState |= (true << 2);  // Третья кнопка: бит 2
            //buttonState |= (true << 3);
            buttonState |= (button_joystick_1 << 3); // Четвертая кнопка: бит 3
            buttonState |= (button_joystick_2 << 4);//  (button_joystick_2 << 4); // Режим управления каждим акутатором отдельно . бит 5
            buttonState |= (button_joystick_3
                            << 5); // Режим управления каждим акутатором отдельно по скорости
            buttonState |= (button_joystick_0 << 6);
            buttonState |= (mISFuseBlown << 7);

            //      bool button0 = (buttonState >> 0) & 1;
            //      bool button1 = (buttonState >> 1) & 1;
            //      bool button2 = (buttonState >> 2) & 1;
            //      bool button3 = (buttonState >> 3) & 1;
            //      bool button4 = (buttonState >> 4) & 1;

            //      qDebug() << "res: " << button_joystick_0 << button_joystick_1 << button_joystick_2 << button_joystick_3;

            ui->horizontalSlider_A->setValue(a);
            ui->horizontalSlider_B->setValue(b);
            ui->horizontalSlider_C->setValue(c);
            ui->horizontalSlider_D->setValue(d);
            //ui->horizontalSlider_E->setValue(e);
            e = ui->horizontalSlider_E->value();
        }

    }
    else if (mControlMode == "Angle-Controls")
    {
        a = ui->horizontalSlider_A->value();
        b = ui->horizontalSlider_B->value();
        c = ui->horizontalSlider_C->value();
        d = ui->horizontalSlider_D->value();
        e = ui->horizontalSlider_E->value();
        f = ui->horizontalSlider_F->value();

        buttonState |= (false << 0); // Первая кнопка: бит 0 (младший бит)
        buttonState |= (false << 1); // Вторая кнопка: бит 1
        buttonState |= (true << 2);  // Третья кнопка: бит 2
        buttonState |= (false << 3); // Четвертая кнопка: бит 3
        buttonState |= (true << 4); // Режим управления каждим акутатором отдельно . бит 5
        buttonState |= (mIsShot << 5); // Режим управления каждим акутатором отдельно по скорости
        buttonState |= (true << 6);
        buttonState |= (mISFuseBlown << 7);

        //  if (SDL_QuitRequested())
        //  {
        //     mTimer->stop();
        //  }

        //  if(SDL_IsGameController(0))
        //  {
        //     float size_diff = 4096;
        //     float size_ang  = 4096;
        //     //          a = (SDL_JoystickGetAxis(joystick, 0) / 32768.f) * size_diff;
        //     //          b =-(SDL_JoystickGetAxis(joystick, 1) / 32768.f) * size_diff;
        //     //          c = (SDL_JoystickGetAxis(joystick, 3) / 32768.f) * size_ang;
        //     //          d = (SDL_JoystickGetAxis(joystick, 4) / 32768.f) * size_ang;
        //     //          e =  0;//SDL_JoystickGetButton(joystick, 6);
        //     //          f =  0;//SDL_JoystickGetButton(joystick, 7);

        //     a = (SDL_JoystickGetAxis(joystick, 0) / 32768.f) * size_diff;
        //     b =-(SDL_JoystickGetAxis(joystick, 4) / 32768.f) * size_diff;
        //     c = (SDL_JoystickGetAxis(joystick, 3) / 32768.f) * size_ang;
        //     d = (SDL_JoystickGetAxis(joystick, 1) / 32768.f) * size_ang;
        //     e =  0;//SDL_JoystickGetButton(joystick, 6);
        //     f =  0;//SDL_JoystickGetButton(joystick, 7);

        //     a = (std::abs(a) < 50) ? 0 : a;
        //     b = (std::abs(b) < 50) ? 0 : b;
        //     c = (std::abs(c) < 50) ? 0 : c;
        //     d = (std::abs(d) < 50) ? 0 : d;
        //     e = (std::abs(e) < 50) ? 0 : e;
        //     f = (std::abs(f) < 50) ? 0 : f;

        //         // float cofficient_angle = (1.f / 1000.f) * ui->horizontalSlider_coff1->value();
        //         // float cofficient_orgin = (1.f / 1000.f) * ui->horizontalSlider_coff2->value();
        //         // angle_yaw += (c * cofficient_angle);
        //         // angle_pitch -= (b * cofficient_angle);
        //         // angle_roll += (a * cofficient_angle);
        //         // orgin_high -= (d * cofficient_orgin);

        //         // ui->dial_angle_yaw->setValue( angle_yaw );
        //         // ui->dial_angle_pitch->setValue( angle_pitch );
        //         // ui->dial_angle_roll->setValue( angle_roll );
        //         // ui->horizontalSlider_HighPosition->setValue( orgin_high );

        //         // qDebug() << a  << b << c << d << angle_yaw << angle_pitch << angle_roll << orgin_high;

        //     //--------------------------------------------//

        //  }
    }


    if (mTimeShot == "single")
    {
        buttonState |= (true << 8);
        buttonState |= (false << 9);
        buttonState |= (false << 10);
    }
    else if (mTimeShot == "two")
    {
        buttonState |= (false << 8);
        buttonState |= (true << 9);
        buttonState |= (false << 10);
    }
    else if (mTimeShot == "all")
    {
        buttonState |= (false << 8);
        buttonState |= (false << 9);
        buttonState |= (true << 10);
    }

    buttonState |= (ui->checkBox_En24->isChecked() << 11);


    //===================================================================//

    //            //-----------------------------------//

    //            a = ui->horizontalSlider_A->value();
    //            b = ui->horizontalSlider_B->value();
    //            c = ui->horizontalSlider_C->value();
    //            d = ui->horizontalSlider_D->value();
    //            e = ui->horizontalSlider_E->value();
    //            f = ui->horizontalSlider_F->value();

    //            //-----------------------------------//

    InterfaceCommunication comm(a, b, c, d, e, f, buttonState);

    if (mSerialPort->isOpen()) {
        mSerialPort->write((char *) &comm, sizeof(InterfaceCommunication));
    } else {
        QString ipAddresStr = ui->lineEdit_IP->text();
        mSocketUDP->writeDatagram((char *) &comm,
                                  sizeof(InterfaceCommunication),
                                  QHostAddress(ipAddresStr),
                                  m_Port);
    }
}

void MainWindow::update_timer()
{
    this->sendCommandToSerialPortAndUDP();
}

void MainWindow::readPendingDatagrams()
{
    while (mSocketUDP->hasPendingDatagrams())
    {
        QByteArray buffer;
        QHostAddress sender;
        quint16 senderPort;
        buffer.resize(mSocketUDP->pendingDatagramSize());
        mSocketUDP->readDatagram(buffer.data(), buffer.size(), &sender, &senderPort);
        //qDebug() << buffer;

        ui->plainTextEdit_Output->appendPlainText(buffer.toStdString().c_str());
    }
}

//------------------------------------------//

void MainWindow::on_pushButton_Connect_clicked()
{
    this->handleConnectAndSendCommand();
}

void MainWindow::on_pushButton_ColsetSerialPort_clicked()
{
    if (mSerialPort)
    {
        //if(mSerialPort->isOpen())
        mSerialPort->close();
    }
    if (mTimer->isActive())
    {
        mTimer->stop();
        ui->pushButton_Timer2->setText("Timer OFF");
       // SDL_JoystickClose(joystick);
        mJoystick.Close();
    }
}

void MainWindow::on_horizontalSlider_A_valueChanged(int value)
{
    ui->spinBox_A->setValue(value);
}

void MainWindow::on_horizontalSlider_B_valueChanged(int value)
{
    ui->spinBox_B->setValue(value);
}

void MainWindow::on_horizontalSlider_C_valueChanged(int value)
{
    ui->spinBox_C->setValue(value);
}

void MainWindow::on_horizontalSlider_D_valueChanged(int value)
{
    ui->spinBox_D->setValue(value);
}

void MainWindow::on_horizontalSlider_E_valueChanged(int value)
{
    ui->spinBox_E->setValue(value);
}

void MainWindow::on_horizontalSlider_F_valueChanged(int value)
{
    ui->spinBox_F->setValue(value);
}

void MainWindow::on_horizontalSlider_A_sliderReleased()
{
    this->sendCommandToSerialPortAndUDP();
}

void MainWindow::on_horizontalSlider_B_sliderReleased()
{
    this->sendCommandToSerialPortAndUDP();
}

void MainWindow::on_horizontalSlider_C_sliderReleased()
{
    this->sendCommandToSerialPortAndUDP();
}

void MainWindow::on_horizontalSlider_D_sliderReleased()
{
    this->sendCommandToSerialPortAndUDP();
}

void MainWindow::on_horizontalSlider_E_sliderReleased()
{
    this->sendCommandToSerialPortAndUDP();
}

void MainWindow::on_horizontalSlider_F_sliderReleased()
{
    this->sendCommandToSerialPortAndUDP();
}

void MainWindow::on_pushButton_SendIP_clicked()
{
    InterfaceCommunication DataCommunication(100, 200, 300, 400, 500, 600);
    QString ipAddresStr = ui->lineEdit_IP->text();
    mSocketUDP->writeDatagram((char *) &DataCommunication,
                              sizeof(InterfaceCommunication),
                              QHostAddress(ipAddresStr),
                              m_Port);
}

void MainWindow::on_pushButton_Timer2_clicked()
{
    if (mTimer->isActive()) {
        mTimer->stop();
        ui->pushButton_Timer2->setText("Timer OFF");
        ui->pushButton_Timer2->setStyleSheet("QPushButton { background-color: white; }");
        SDL_JoystickClose(joystick);

        mJoystick.Close();

    } else {
        mTimer->start(50);
        ui->pushButton_Timer2->setText("Timer ON");
        ui->pushButton_Timer2->setStyleSheet("QPushButton { background-color: red; }");
    }
}

void MainWindow::on_comboBox_mode_currentTextChanged(const QString &arg)
{
    qDebug() << arg;
    mControlMode = arg;

    if (arg == "InvidualControls-Position" || arg == "InvidualControls-Velocity"
        || arg == "Joystik-Controls") {
        ui->horizontalSlider_A->setMinimum(-10000);
        ui->horizontalSlider_A->setMaximum(10000);

        ui->horizontalSlider_B->setMinimum(-10000);
        ui->horizontalSlider_B->setMaximum(10000);

        ui->horizontalSlider_C->setMinimum(-10000);
        ui->horizontalSlider_C->setMaximum(10000);

        ui->horizontalSlider_D->setMinimum(-10000);
        ui->horizontalSlider_D->setMaximum(10000);

        ui->horizontalSlider_E->setMinimum(-10000);
        ui->horizontalSlider_E->setMaximum(10000);

        ui->horizontalSlider_F->setMinimum(-10000);
        ui->horizontalSlider_F->setMaximum(10000);
    }

    if (arg == "Joystik-Controls" || arg == "Angle-Controls") {
        if (!joystick) {
            joystick = JoystickInit(0);
            ui->plainTextEdit_Output->appendPlainText("Joystick-Connect - YES \n");
        }

        m_isInitJoysick = (joystick) ? true : false;
        if (m_isInitJoysick) {
            ui->label_Status->setText("Joystick Initilization");
        }
    } else {
        if (joystick != NULL) {
            SDL_JoystickClose((joystick));
            m_isInitJoysick = false;
            joystick = NULL;
        }
    }
}

void MainWindow::on_dial_angle_yaw_valueChanged(int value)
{
    ui->label_yaw->setText(QString::number(value / 1000.f));
}

void MainWindow::on_dial_angle_pitch_valueChanged(int value)
{
    ui->label_pitch->setText(QString::number(value / 1000.f));
}

void MainWindow::on_dial_angle_roll_valueChanged(int value)
{
    ui->label_roll->setText(QString::number(value / 1000.f));
}

//void MainWindow::on_pushButton_Reset_clicked()
//{
//    //    a = 0;
//    //    b = 0;
//    //    c = 0;
//    //    d = 0;
//    //    e = 0;
//    //    f = 0;
//    //    buttonState |= (true << 0); //
//    //    buttonState |= (true << 1); //
//    //    buttonState |= (true << 2); //
//    //    buttonState |= (true << 3); //

//    //    InterfaceCommunication comm(a,b,c,d,e,f, buttonState);

//    //    if(mSerialPort->isOpen())
//    //    {
//    //       mSerialPort->write((char*)&comm, sizeof(InterfaceCommunication));
//    //    }
//    //    else
//    //    {
//    //       QString ipAddresStr = ui->lineEdit_IP->text();
//    //       mSocketUDP->writeDatagram( (char*)&comm, sizeof(InterfaceCommunication), QHostAddress(ipAddresStr), m_Port);
//    //    }

//    SDL_JoystickClose(0);
//}

void MainWindow::on_pushButton_Shot_pressed()
{
    mIsShot = true;
}

void MainWindow::on_pushButton_Shot_released()
{
    mIsShot = false;
}

void MainWindow::on_spinBox_A_valueChanged(int arg1)
{
    //ui->horizontalSlider_A->setValue(arg1);
}

void MainWindow::on_spinBox_B_valueChanged(int arg1)
{
    //ui->horizontalSlider_B->setValue(arg1);
}

void MainWindow::on_spinBox_C_valueChanged(int arg1)
{
    //ui->horizontalSlider_C->setValue(arg1);
}

void MainWindow::on_spinBox_D_valueChanged(int arg1)
{
    //ui->horizontalSlider_D->setValue(arg1);
}

void MainWindow::on_spinBox_E_valueChanged(int arg1)
{
    //ui->horizontalSlider_E->setValue(arg1);
}

void MainWindow::on_spinBox_F_valueChanged(int arg1)
{
    //ui->horizontalSlider_F->setValue(arg1);
}

void MainWindow::on_spinBox_F_editingFinished()
{
    ui->horizontalSlider_F->setValue(ui->spinBox_F->value());
    this->sendCommandToSerialPortAndUDP();
}

void MainWindow::on_spinBox_E_editingFinished()
{
    ui->horizontalSlider_E->setValue(ui->spinBox_E->value());
    this->sendCommandToSerialPortAndUDP();
}

void MainWindow::on_spinBox_D_editingFinished()
{
    ui->horizontalSlider_D->setValue(ui->spinBox_D->value());
    this->sendCommandToSerialPortAndUDP();
}

void MainWindow::on_spinBox_C_editingFinished()
{
    ui->horizontalSlider_C->setValue(ui->spinBox_C->value());
    this->sendCommandToSerialPortAndUDP();
}

void MainWindow::on_spinBox_B_editingFinished()
{
    ui->horizontalSlider_B->setValue(ui->spinBox_B->value());
    this->sendCommandToSerialPortAndUDP();
}

void MainWindow::on_spinBox_A_editingFinished()
{
    ui->horizontalSlider_A->setValue(ui->spinBox_A->value());
    this->sendCommandToSerialPortAndUDP();
}

void MainWindow::on_checkBox_JoystickIntegrateAngle_toggled(bool checked)
{
    mISControl = checked;
}

void MainWindow::on_pushButton_Clear_clicked()
{
    ui->plainTextEdit_Output->clear();
}

void MainWindow::on_pushButton_JoystikOn_clicked()
{
    if (m_isInitJoysick == true) {
        SDL_JoystickClose(joystick);
        ui->pushButton_JoystikOn->setText("Joystick OFF");
        m_isInitJoysick = false;
    } else {
        joystick = JoystickInit(0);
        m_isInitJoysick = (joystick) ? true : false;
        if (m_isInitJoysick) {
            ui->pushButton_JoystikOn->setText("Joystick Initilization");
        }
    }
}

void MainWindow::on_checkBox_FuseBlown_toggled(bool checked)
{
    mISFuseBlown = checked;
}

void MainWindow::on_dial_angle_yaw_sliderReleased()
{
    qDebug() << "Dial Angle";
    this->sendCommandToSerialPortAndUDP();
}

void MainWindow::on_comboBox_ShotTime_currentTextChanged(const QString &arg1)
{
    mTimeShot = arg1;
}

void MainWindow::on_checkBox_invert_toggled(bool checked)
{
    invert = (checked) ? -1 : 1;
}

void MainWindow::on_comboBox_TimeShot_currentTextChanged(const QString &arg1)
{
    mTimeShot = arg1;
}
