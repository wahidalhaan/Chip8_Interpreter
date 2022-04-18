#include "SFML/Graphics.hpp"
#include <algorithm>
#include <fstream>
#include <functional>
#include <unordered_map>
#include "chip8.h"
void Chip8::initialize() {
  PC = 0x200;
  V.fill(0);
  stack.fill(0);
  display.fill(0);
  input.fill(0);
  memory.fill(0);
  opcode = 0x000;
  I = 0x000;
  stack_pointer = 0x000;
  drawFlag = false;
  delay_timer = sound_timer = 0;
  for (int i = 0; i < 80; i++)
    memory[i] = chip8_fontset[i];
}
void Chip8::loadRom(const char *path) {

  std::ifstream ifs(path, std::ios::in | std::ios::binary);
  if(!ifs.is_open())
    printf("cannot open file\n");
  ifs.seekg(0, ifs.end);
  const int size = ifs.tellg();
  ifs.seekg(0, ifs.beg);
  uint8_t *code = new uint8_t[size];
  ifs.read((char *)code, size);
  for (int i = 0; i < size; i++)
    memory[i + 512] = code[i];
  delete[] code;
  return;
}
void Chip8::emulateCycle() {
#define Vx V[((opcode & 0x0F00) >> 8)]
#define Vy V[((opcode & 0x00F0) >> 4)]
#define _NNN (opcode & 0x0FFF)
#define _NN (opcode & 0x00FF)
  const static auto checkOverFlow = [](uint8_t x, uint8_t y) -> bool {
    return ((0xFF - x) < y);
  };
  const static auto checkUnderFlow = [](uint8_t x, uint8_t y) -> bool {
    return x < y;
  };
  const static auto WaitForInputAndAssignToVx = [&]() {
    bool pressed = false;
    for (int i = 0; i < 16; i++)
      if (input[i] == 1) {
        pressed = true;
        Vx = i;
      }
    if (!pressed)
      return;
    PC += 2;
  };

  const static auto DumpVXToMemory = [&]() {
    const uint16_t regmax = ((opcode & 0x0F00) >> 8);
    for (unsigned int i = 0; i <= regmax; i++)
      memory[I + i] = V[i];
  };
  const static auto DumpMemoryToVx = [&]() {
    const unsigned int regmax = ((opcode & 0x0F00) >> 8);
    for (unsigned int i = 0; i <= regmax; i++)
      V[i] = memory[I + i];
  };
  const static auto DrawSprites = [&]() {
    const size_t rows = (opcode & 0x000F);
    const size_t cols = 8;
    const size_t x = Vx % 64;
    const size_t y = Vy % 32;
    V[0xF] = 0;
    for (unsigned int i = 0; i < rows; i++) {
      unsigned char sprite_data = memory[I + i];
      for (unsigned int j = 0; j < cols; j++) {
        if ((sprite_data & (0x80 >> j)) != 0) {
          if (display[(x + j + ((y + i) * 64))] == 1) {
            V[0xF] = 1;
          }
          display[x + j + ((y + i) * 64)] ^= 1;
        }
      }
    }
    drawFlag = true;
    PC += 2;
  };
  static const std::unordered_map<uint16_t, std::function<void(void)>> map = {
      {0x0000,
       [&]() {
         display.fill(0);
         PC += 2;
       }},
      {0x000E,
       [&]() {
         --stack_pointer;
         PC = stack[stack_pointer];
         PC += 2;
       }},
      {0x1000, [&]() { PC = _NNN; }},
      {0x2000,
       [&]() {
         stack[stack_pointer++] = PC;
         PC = _NNN;
       }},
      {0x3000, [&]() { Vx == _NN ? PC += 4 : PC += 2; }},
      {0x4000, [&]() { Vx != _NN ? PC += 4 : PC += 2; }},
      {0x5000, [&]() { Vx == Vy ? PC += 4 : PC += 2; }},
      {0x6000,
       [&]() {
         Vx = _NN;
         PC += 2;
       }},
      {0x7000,
       [&]() {
         Vx += _NN;
         PC += 2;
       }},
      {0x8000,
       [&]() {
         Vx = Vy;
         PC += 2;
       }},
      {0x8001,
       [&]() {
         Vx |= Vy;
         PC += 2;
       }},
      {0x8002,
       [&]() {
         Vx &= Vy;
         PC += 2;
       }},
      {0x8003,
       [&]() {
         Vx ^= Vy;
         PC += 2;
       }},
      {0x8004,
       [&]() {
         V[0xF] = checkOverFlow(Vx, Vy) ? 1 : 0;
         Vx += Vy;
         PC += 2;
       }},
      {0x8005,
       [&]() {
         V[0xF] = checkUnderFlow(Vx, Vy) ? 0 : 1;
         Vx -= Vy;
         PC += 2;
       }},
      {0x8006,
       [&]() {
         V[0xF] = Vx & 0x1;
         Vx >>= 1;
         PC += 2;
       }},
      {0x8007,
       [&]() {
         V[0xF] = Vx < Vy ? 1 : 0;
         Vx = Vy - Vx;
         PC += 2;
       }},
      {0x800E,
       [&]() {
         if (Vx & 0x80)
           V[0xF] = 1;
         Vx <<= 1;
         PC += 2;
       }},
      {0x9000,
       [&]() {
         if (Vx != Vy)
           PC += 2;
         PC += 2;
       }},
      {0xA000,
       [&]() {
         I = _NNN;
         PC += 2;
       }},
      {0xB000, [&]() { PC = V[0] + _NNN; }},
      {0xC000,
       [&]() {
         Vx = ((rand() % 0xFF) & _NN);
         PC += 2;
       }},
      {0xD000, DrawSprites},
      {0xE00E, [&]() { PC += input[Vx] != 0 ? 4 : 2; }},
      {0xE001, [&]() { PC += input[Vx] == 0 ? 4 : 2; }},
      {0xF007,
       [&]() {
         Vx = delay_timer;
         PC += 2;
       }},
      {0xF00A, WaitForInputAndAssignToVx},
      {0xF005,
       [&]() {
         if (_NN == 0x0015)
           delay_timer = Vx;
         else if (_NN == 0x0055)
           DumpVXToMemory();
         else
           DumpMemoryToVx();
         PC += 2;
       }},
      {0xF008,
       [&]() {
         sound_timer = Vx;
         PC += 2;
       }},
      {0xF00E,
       [&]() {
         I += Vx;
         PC += 2;
       }},
      {0xF009,
       [&]() {
         I = Vx * 0x5;
         PC += 2;
       }},
      {0xF003, [&]() {
         memory[I] = Vx / 100;
         memory[I + 1] = (Vx / 10) % 10;
         memory[I + 2] = Vx % 10;
         PC += 2;
       }}};

  opcode = ((memory[PC] << 8) | (memory[PC + 1]));
  constexpr static std::array<uint16_t, 4> arr{0x0000, 0x0008, 0x000F, 0x000E};
  if (std::any_of(arr.begin(), arr.end(),
                  [&](uint16_t i) { return i == ((opcode & 0xF000) >> 12); }))
    map.at((opcode & 0xF00F))();
  else
    map.at(opcode & 0xF000)();

  if (delay_timer > 0)
    --delay_timer;
  if (sound_timer > 0)
    --sound_timer;
}

void Chip8::displayGraphics(sf::Texture &gfx) {
  if (drawFlag) {
    drawFlag = false;
    uint8_t color[64 * 32 * 4] = {};
    for (int i = 0; i < 64 * 32; i++) {
      int j = i * 4;
      uint8_t pixel = display[i];
      color[j] = pixel * 255;
      color[j + 1] = color[j + 2] = pixel * 255;
      color[j + 3] = 255;
    }
    gfx.update(color);
  }
}
void Chip8::getInput(sf::RenderWindow &window, sf::Event &evnt) {
  while (window.pollEvent(evnt)) {
    if (evnt.type == sf::Event::Closed)
      window.close();
    else if (evnt.type == sf::Event::KeyPressed) {
      for (unsigned int i = 0; i < 16; i++)
        if (evnt.key.code == sf_input[i])
          input[i] = 1;
    } else if (evnt.type == sf::Event::KeyReleased) {
      for (unsigned int i = 0; i < 16; i++)
        if (evnt.key.code == sf_input[i])
          input[i] = 0;
    }
  }
}
