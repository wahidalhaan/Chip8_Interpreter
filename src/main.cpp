#include "chip8.h"
#include <SFML/Graphics.hpp>
#include <cstdio>

sf::Texture gfx;
sf::RenderWindow window(sf::VideoMode(640, 320), "Chip8 Interpreter");
sf::Sprite display;


int main(int argc,char **argv) {


  if (argc != 2)
  {
        std::printf("Usage :\n./Chip8_Interpreter.exe RomPath\n");
        getchar();
        exit(0);
  }
  Chip8 chip8;
  chip8.initialize();
  chip8.loadRom(argv[1]);

  gfx.create(64, 32);
  display.setTexture(gfx);
  display.setScale(10.0f, 10.0f);
  while (window.isOpen()) {
    sf::Event evnt;
    chip8.emulateCycle();

    chip8.getInput(window, evnt); 
    chip8.displayGraphics(gfx);
    window.clear();
    window.draw(display);
    window.display();
    sf::sleep(sf::microseconds(1200));
  }

  return 0;
}
