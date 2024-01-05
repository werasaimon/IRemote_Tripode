#ifndef IJOYSTICK_H
#define IJOYSTICK_H

#include <QObject>
#include <SDL2/SDL.h>
#include <SDL2/SDL_joystick.h>

class IJoystick
{

public:
    explicit IJoystick();
    ~IJoystick();

    void Close();

    SDL_Joystick* Initilization(int index_joystick);
    int16_t GetAxisValue(const int& index);
    bool GetButton(const int& index);

    bool IsGameController(const int& index);

    SDL_Joystick *Get_SDLJoystick() const;

    std::string getName() const;
    int num_axes() const;
    int num_buttons() const;
    int num_hats() const;

signals:

private:

    _SDL_Joystick *mSDL_Joystick;
   //--------------//
   std::string mName;
   int mNum_axes;
   int mNum_buttons;
   int mNum_hats;
};

#endif // IJOYSTICK_H
