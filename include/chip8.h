#include "SFML/Graphics.hpp"
#include <array>
#include <fstream>
#include <string.h>
constexpr static size_t REGISTER_SIZE = 16;
constexpr static size_t MEMORY_SIZE = 4096;
constexpr static size_t STACK_SIZE = 16;
constexpr static size_t INPUT_SIZE = 16;
constexpr static size_t DISPLAY_SIZE = 32 * 64;

class Chip8 {
public:
  std::array<uint8_t, MEMORY_SIZE> memory;
  /** the interpretor itself occupies the first 512 bytes(0x200) so instead
      we can load the font set there
      and start after 512 bytes **/
  std::array<uint8_t, REGISTER_SIZE> V;
  /** VF is the carry flag in addition or no borow flag in substraction and is
   * set upon pixel collision. **/

  uint16_t I; // address register used with opcodes.

  uint16_t PC; // program counter

  std::array<uint16_t, STACK_SIZE> stack;

  uint16_t stack_pointer;

  uint8_t delay_timer; // intended for use in timing events
  uint8_t sound_timer; // beeps when value is non zero

  std::array<uint8_t, INPUT_SIZE> input;
  constexpr static std::array<sf::Keyboard::Key, INPUT_SIZE> sf_input = {
      sf::Keyboard::X,    sf::Keyboard::Num1, sf::Keyboard::Num2,
      sf::Keyboard::Num3, sf::Keyboard::Q,    sf::Keyboard::W,
      sf::Keyboard::E,    sf::Keyboard::A,    sf::Keyboard::S,
      sf::Keyboard::D,    sf::Keyboard::Z,    sf::Keyboard::C,
      sf::Keyboard::Num4, sf::Keyboard::R,    sf::Keyboard::F,
      sf::Keyboard::V};

  std::array<uint8_t, DISPLAY_SIZE> display;

  uint16_t opcode; // holds the value for the current opcode
  bool drawFlag;
  // unsigned char chip8_fontset[80] =
  constexpr static std::array<uint8_t, 80> chip8_fontset = {
      0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
      0x20, 0x60, 0x20, 0x20, 0x70, // 1
      0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
      0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
      0x90, 0x90, 0xF0, 0x10, 0x10, // 4
      0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
      0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
      0xF0, 0x10, 0x20, 0x40, 0x40, // 7
      0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
      0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
      0xF0, 0x90, 0xF0, 0x90, 0x90, // A
      0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
      0xF0, 0x80, 0x80, 0x80, 0xF0, // C
      0xE0, 0x90, 0x90, 0x90, 0xE0, // D
      0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
      0xF0, 0x80, 0xF0, 0x80, 0x80  // F
  };
  void initialize();
  void loadRom(const char *path);
  Chip8() = default;
  Chip8(Chip8 &) = delete;
  Chip8(Chip8 &&) = delete;

  void emulateCycle();
  void displayGraphics(sf::Texture &gfx);
  void getInput(sf::RenderWindow &window, sf::Event &evnt);
};
