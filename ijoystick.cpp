#include "ijoystick.h"

IJoystick::IJoystick()
{}

IJoystick::~IJoystick()
{
    SDL_JoystickClose(mSDL_Joystick);
    SDL_Quit();
}

void IJoystick::Close()
{
    SDL_JoystickClose(mSDL_Joystick);
    mSDL_Joystick = nullptr;
}

SDL_Joystick *IJoystick::Initilization(int index_joystick)
{
    // Инициализация SDL для использования джойстика
    SDL_Init(SDL_INIT_JOYSTICK);
    // Включаем
    SDL_JoystickEventState(SDL_ENABLE);

    // If there are no joysticks connected, quit the program
    if (SDL_NumJoysticks() <= 0)
    {
        printf("There are no joysticks connected. Quitting now...\n");
        SDL_Quit();
    }

    // Open the joystick for reading and store its handle in the joy variable
    mSDL_Joystick = SDL_JoystickOpen(index_joystick);

    //----------------------------------------------------//

    if (mSDL_Joystick != NULL)
    {
        // Get information about the joystick
        mName = SDL_JoystickName(mSDL_Joystick);
        mNum_axes = SDL_JoystickNumAxes(mSDL_Joystick);
        mNum_buttons = SDL_JoystickNumButtons(mSDL_Joystick);
        mNum_hats = SDL_JoystickNumHats(mSDL_Joystick);

        printf("Now reading from joystick '%s' with:\n"
               "%d axes\n"
               "%d buttons\n"
               "%d hats\n\n",
               mName.c_str(),
               mNum_axes,
               mNum_buttons,
               mNum_hats);
    }

    return mSDL_Joystick;

}

int16_t IJoystick::GetAxisValue(const int &index)
{
    return SDL_JoystickGetAxis(mSDL_Joystick, index);
}

bool IJoystick::GetButton(const int &index)
{
    return SDL_JoystickGetButton(mSDL_Joystick, index);
}

bool IJoystick::IsGameController(const int &index)
{
    return IsGameController(index);
}

_SDL_Joystick *IJoystick::Get_SDLJoystick() const
{
    return mSDL_Joystick;
}

std::string IJoystick::getName() const
{
    return mName;
}

int IJoystick::num_axes() const
{
    return mNum_axes;
}

int IJoystick::num_buttons() const
{
    return mNum_buttons;
}

int IJoystick::num_hats() const
{
    return mNum_hats;
}
