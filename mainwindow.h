#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QTimer>
#include <QCloseEvent>
#include <QUdpSocket>
#include <SDL2/SDL.h>
//#include <QtGamepad/qgamepad.h>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE



//struct InterfaceCommunication
//{
//    InterfaceCommunication(int _a = 0, int _b = 0, int _c = 0, int _d = 0, int _e = 0, int _f = 0, char _buttonState = 0)
//        : a(_a), b(_b), c(_c), d(_d), e(_e), f(_f), buttonState(_buttonState)
//    {
//    }

//    int16_t a;
//    int16_t b;
//    int16_t c;
//    int16_t d;
//    int16_t e;
//    int16_t f;
//    int16_t buttonState;

//};


struct alignas(16) InterfaceCommunication
{
    InterfaceCommunication(int16_t _a = 0, int16_t _b = 0, int16_t _c = 0,
                           int16_t _d = 0, int16_t _e = 0, int16_t _f = 0 , int16_t _buttonState = 0)
        : a(_a), b(_b), c(_c), d(_d), e(_e), f(_f), buttonState(_buttonState)
    {
    }

    int16_t a;
    int16_t b;
    int16_t c;
    int16_t d;
    int16_t e;
    int16_t f;
    int16_t buttonState;
};

/**
// Пример класса, который будет сериализован и передан по последовательному порту
class MyClass {
public:
    int value1;
    float value2;
    std::string value3;

    // Метод для получения размера объекта класса в байтах
    int getSizeInBytes() const
    {
        int size = sizeof(value1) + sizeof(value2) + value3.size();
        return size;
    }

    // Метод сериализации объекта в байты
    void serialize(uint8_t* buffer) const
    {
        int offset = 0;

        // Сериализация value1
        memcpy(buffer + offset, &value1, sizeof(value1));
        offset += sizeof(value1);

        // Сериализация value2
        memcpy(buffer + offset, &value2, sizeof(value2));
        offset += sizeof(value2);

        // Сериализация value3
        memcpy(buffer + offset, value3.c_str(), value3.size());
    }

    // Метод десериализации байтов в объект класса
    void deserialize(const uint8_t* buffer)
    {
        int offset = 0;

        // Десериализация value1
        memcpy(&value1, buffer + offset, sizeof(value1));
        offset += sizeof(value1);

        // Десериализация value2
        memcpy(&value2, buffer + offset, sizeof(value2));
        offset += sizeof(value2);

        // Десериализация value3
        char value3Buffer[256]; // Предполагается, что value3 не будет длиннее 256 символов
        memcpy(value3Buffer, buffer + offset, value3.size());
        value3Buffer[value3.size()] = '\0';
        value3 = value3Buffer;
    }
};
/**/

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


    void sendCommandToSerialPortAndUDP();
    void closeEvent(QCloseEvent *event);

    SDL_Joystick *JoystickInit(int index_joystick);
    void JoystickClose(SDL_Joystick *joystick);

private slots:

    void handleReadSerialData();
    void handleConnectAndSendCommand();
    void update_timer();

    void readPendingDatagrams();

    void on_pushButton_Connect_clicked();

    void on_pushButton_ColsetSerialPort_clicked();

    void on_horizontalSlider_A_valueChanged(int value);

    void on_horizontalSlider_B_valueChanged(int value);

    void on_horizontalSlider_C_valueChanged(int value);

    void on_horizontalSlider_A_sliderReleased();

    void on_horizontalSlider_B_sliderReleased();

    void on_horizontalSlider_C_sliderReleased();


    void on_horizontalSlider_D_valueChanged(int value);

    void on_horizontalSlider_E_valueChanged(int value);

    void on_horizontalSlider_F_valueChanged(int value);

    void on_horizontalSlider_D_sliderReleased();

    void on_horizontalSlider_E_sliderReleased();

    void on_horizontalSlider_F_sliderReleased();

    void on_pushButton_SendIP_clicked();

    void on_pushButton_Timer2_clicked();

    void on_comboBox_mode_currentTextChanged(const QString &arg);

    void on_dial_angle_yaw_valueChanged(int value);

    void on_dial_angle_pitch_valueChanged(int value);

    void on_dial_angle_roll_valueChanged(int value);

    //void on_pushButton_Reset_clicked();


    void on_pushButton_Shot_pressed();

    void on_pushButton_Shot_released();

    void on_spinBox_A_valueChanged(int arg1);

    void on_spinBox_B_valueChanged(int arg1);

    void on_spinBox_C_valueChanged(int arg1);

    void on_spinBox_D_valueChanged(int arg1);

    void on_spinBox_E_valueChanged(int arg1);

    void on_spinBox_F_valueChanged(int arg1);

    void on_spinBox_F_editingFinished();

    void on_spinBox_E_editingFinished();

    void on_spinBox_D_editingFinished();

    void on_spinBox_C_editingFinished();

    void on_spinBox_B_editingFinished();

    void on_spinBox_A_editingFinished();

    void on_checkBox_JoystickIntegrateAngle_toggled(bool checked);

    void on_pushButton_Clear_clicked();

    void on_pushButton_JoystikOn_clicked();

    void on_checkBox_FuseBlown_toggled(bool checked);

    void on_dial_angle_yaw_sliderReleased();

    void on_comboBox_ShotTime_currentTextChanged(const QString &arg1);

    void on_checkBox_invert_toggled(bool checked);

    void on_comboBox_TimeShot_currentTextChanged(const QString &arg1);

private:

    Ui::MainWindow *ui;

    //-----------------------//
    QString mControlMode;
    //------------------------//
    QSerialPort* mSerialPort;
    QSerialPort* mSerialPortInfo;
    //------------------------//
    QString mSerialPortName;
    //-----------------------//
    QTimer *mTimer;
    //-----------------------//
    // Порт обмена данными
    quint16 m_Port;
    QUdpSocket *mSocketUDP;
    //-----------------------//

    bool m_isInitJoysick;
    SDL_Joystick *joystick;
    const char *name;
    int num_axes;
    int num_buttons;
    int num_hats;
    //-----------------------//
    bool mIsShot;
    bool mISControl;
    bool mISFuseBlown;

    //-----------------------//

    QString mTimeShot;

    //-----------------------//

    float angle_yaw;
    float angle_pitch;
    float angle_roll;
    float orgin_high;

    //-----------------------//

    int invert;

};
#endif // MAINWINDOW_H
