#pragma once

#include <Kore/Graphics2/Graphics.h>

using namespace Kore;

namespace {
	const char* tileFile;
	Graphics4::Texture* image;
	
	const int lightCount = 8;
	const int spiderCountMax = 16;
	const int tileWidth = 128;
	const int tileHeight = 168;
	const int spiderCooldownMax = 30;

	int spiderFrameCount = 0;
	vec2i spiderPos[spiderCountMax];
	int spiderState[spiderCountMax];
	int spiderCooldownCurr[spiderCountMax];
	int spiderCountCurr;
	
	const int rows = 3;
	const int columns = 6;
	
	int* source;

	vec2* doors;
	int doorCount;
}

void loadCsv(const char* csvFile);

void initTiles(const char* csvFile, const char* tileFile);
void drawTiles(Graphics2::Graphics2* g2, float camX, float camY);
void shuffleDoors();

int getFloor(float py);
int getTileID(float px, float py);
int getTileIndex(float px, float py);
vec2 findDoor(float lastX, float lastY);

void resetSpiders();
bool animateSpider(float px, float py, float fx, float fy, float mx_world, float my_world, float energy);
	
enum TileID {Door = 0, Window = 1, Books = 2, Closet = 3, TableGlobus = 4, TableAndCandles = 5, SpiderWeb = 6, Spider1 = 7, Spider2 = 8, Spider3 = 9, Spider4 = 10, Spider5 = 11, Spider6 = 12, Spider7 = 13, Spider8 = 14, BookShelf = 15, Wall = 16, Heater = 17, Laptop = 18, PC = 19, DrinkAutomat = 20, Pillar = 21, TV = 22, Plant1 = 23, PlantAndClock = 24, Candle = 25, BottleAndLaptop = 26, CactusAndPictures = 27, Stairs1 = 28, Stairs2 = 29, Stairs3 = 30, Stairs4 = 31, Stairs5 = 32, Stairs6 = 33};
