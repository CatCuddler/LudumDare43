#include "pch.h"

#include <Kore/IO/FileReader.h>
#include <Kore/Graphics4/Graphics.h>
#include <Kore/Input/Keyboard.h>
#include <Kore/System.h>
#include <Kore/Log.h>
#include <Kore/Graphics1/Color.h>

#include "Tileset.h"
#include "Animation.h"

using namespace Kore;

namespace {
	const int scale = 4;
	const int w = 128 * 2;
	const int h = 168;
	
	float px = 0;
	float py = 0;
	float camX = 0;
	float camY = 0;
	
	int level = 1;
	
	Graphics2::Graphics2* g2;
	
	Animation* cat_walk;
	Animation* cat_jump;
	Animation* cat_attack;
	vec2 playerCenter;
	float playerWidth, playerHeight;
	
	float targetYPosition;
	
	Animation* guy;
	vec2 guyPosition;
	
	int lastDirection = 1;	// 0 - left, 1 - right
	bool left, right, up, down, jump, attack;
	
	Kravur* font14;
	Kravur* font24;
	Kravur* font34;
	Kravur* font44;
	
	const char* helpText;
	const char* const stairsUp = "Key Up: walk the stairs up";
	const char* const stairsDown = "Key Down: walk the stairs down";
	const char* const jumpText = "Key Up: jump on the table";
	const char* const loadLevelText = "Key Up: load next level";
	
	enum GameState {
		TitleState, InGameState, GameOverState
	};
	GameState state;
	
	void loadNextLevel() {
		char levelName[20];
		sprintf(levelName, "Tiles/level%i.csv", level);
		log(LogLevel::Info, "Load level %i", level);
		initTiles(levelName, "Tiles/tiles.png");
		++level;
	}
	
	void moveCatInTheMiddleOfTheTile() {
		vec2 tileCenter = getTileCenterBottom(playerCenter.x(), playerCenter.y());
		px = tileCenter.x() - playerWidth / 2;
		py = tileCenter.y() - playerHeight;
	}
	
	void moveCat() {
		int tileID = getTileID(playerCenter.x(), playerCenter.y());
		//log(LogLevel::Info, "%i", tileID);
		
		helpText = nullptr;
		if (tileID == Stairs3) {
			//log(LogLevel::Info, "walk downstairs -> left");
			helpText = stairsDown;
		}
		else if (tileID == Stairs4) {
			//log(LogLevel::Info, "walk downstairs -> right");
			helpText = stairsDown;
		}
		else if (tileID == Stairs1) {
			//log(LogLevel::Info, "walk upstairs -> right");
			helpText = stairsUp;
		}
		else if (tileID == Stairs6) {
			//log(LogLevel::Info, "walk upstairs -> left");
			helpText = stairsUp;
		}
		// Check if next level can be load
		else if (tileID == Door) {
			helpText = loadLevelText;
		}
		
		// Check if the cat can jump on the table
		if (cat_walk->status == Animation::Status::WalkingRight)
			tileID = getTileID(playerCenter.x() + tileWidth, playerCenter.y());
		if (cat_walk->status == Animation::Status::WalkingLeft)
			tileID = getTileID(playerCenter.x() - tileWidth, playerCenter.y());
		if (tileID == TableGlobus1 || tileID == TableAndCandles) {
			helpText = jumpText;
		}
		
		float moveDistance = 4;
		
		if (cat_walk->status != Animation::Status::WalkingDownLeft && cat_walk->status != Animation::Status::WalkingDownRight &&
			cat_walk->status != Animation::Status::WalkingUpLeft && cat_walk->status != Animation::Status::WalkingUpRight) {
			if (left && px >= -10) {
				px -= moveDistance;
				cat_walk->status = Animation::Status::WalkingLeft;
			} else if (right && px <= columns * tileWidth - playerWidth + 10) {
				px += moveDistance;
				cat_walk->status = Animation::Status::WalkingRight;
			} else if (up /*&& py >= tileHeight - playerHeight + moveDistance*/) {
				if (tileID == Stairs1) {
					moveCatInTheMiddleOfTheTile();
					targetYPosition = py - tileHeight;
					cat_walk->status = Animation::Status::WalkingUpRight;
				}
				if (tileID >= Stairs6) {
					moveCatInTheMiddleOfTheTile();
					px += 50;
					targetYPosition = py - tileHeight;
					cat_walk->status = Animation::Status::WalkingUpLeft;
				}
				if (tileID == Door) {
					loadNextLevel();
				}
			} else if (down /*&& py <= rows * tileHeight - playerHeight*/) {
				if (tileID == Stairs3) {
					moveCatInTheMiddleOfTheTile();
					targetYPosition = py + tileHeight;
					cat_walk->status = Animation::Status::WalkingDownLeft;
				}
				if (tileID == Stairs4) {
					moveCatInTheMiddleOfTheTile();
					targetYPosition = py + tileHeight;
					cat_walk->status = Animation::Status::WalkingDownRight;
				}
			}
		}
		//log(LogLevel::Info, "%f %f", playerPosition.x(), playerPosition.y());
		
		if (cat_walk->status == Animation::Status::WalkingDownLeft) {
			px -= moveDistance;
			py += moveDistance;
			if (py == targetYPosition) cat_walk->status = Animation::Status::StandingLeft;
		}
		if (cat_walk->status == Animation::Status::WalkingUpRight) {
			px += moveDistance;
			py -= moveDistance;
			if (py == targetYPosition) cat_walk->status = Animation::Status::StandingRight;
		}
		if (cat_walk->status == Animation::Status::WalkingDownRight) {
			px += moveDistance;
			py += moveDistance;
			if (py == targetYPosition) cat_walk->status = Animation::Status::StandingRight;
		}
		if (cat_walk->status == Animation::Status::WalkingUpLeft) {
			px -= moveDistance;
			py -= moveDistance;
			if (py == targetYPosition) cat_walk->status = Animation::Status::StandingLeft;
		}
		
		playerCenter = vec3(px + playerWidth / 2, py + playerHeight / 2);
		cat_walk->update(playerCenter);
	}
	
	void moveGuy() {
		// TODO: guy should follow the cat
		guyPosition = playerCenter;
		
		guy->update(guyPosition);
	}
	
	void drawGUI() {
		g2->setFont(font14);
		g2->setFontColor(Graphics1::Color::Black);
		g2->setFontSize(14);
		
		if (helpText != nullptr) {
			g2->drawString(helpText, 0, 0);
		}
		
		g2->setColor(Graphics1::Color::White);
	}

	void update() {
		Graphics4::begin();
		Graphics4::restoreRenderTarget();
		Graphics4::clear(Graphics4::ClearColorFlag);
		
		float targetCamX = Kore::min(Kore::max(0.0f, playerCenter.x() - w / 2), 1.f * columns * tileWidth - w);
		float targetCamY = playerCenter.y() - tileHeight + playerHeight / 2;//Kore::min(Kore::max(0.0f, playerCenter.y() - h / 2), 1.f * rows * tileHeight - h);
		
		vec2 cam(camX, camY);
		vec2 target(targetCamX, targetCamY);
		vec2 dir = target - cam;
		if (dir.getLength() < 16.0f) {
			camX = targetCamX;
			camY = targetCamY;
		} else {
			dir.setLength(15.0f);
			cam = cam + dir;
			camX = cam.x();
			camY = cam.y();
		}
		
		moveCat();
		//moveGuy();

		g2->begin(false, w, h);
		
		if (state == TitleState) {
			log(LogLevel::Info, "Add title screen");
		} else if (state == InGameState) {
			//camX = playerPosition.x();
			//camY = playerPosition.y();
			drawTiles(g2, camX, camY);
			
			cat_walk->render(g2, camX, camY);
			//guy->render(g2);
			
			animateSpider(playerCenter.x(), playerCenter.y());
			animateGlobus(playerCenter.x(), playerCenter.y());
			
			drawGUI();
		} else if (state == GameOverState) {
			log(LogLevel::Info, "Add game over screen");
			//g2->drawImage(gameOverImage, 0, 0);
		}
		
		
		
		g2->end();

		Graphics4::end();
		Graphics4::swapBuffers();
	}
	
	void keyDown(KeyCode code) {
		switch (code) {
			case KeyLeft:
			case KeyA:
				left = true;
				lastDirection = 0;
				break;
			case KeyRight:
			case KeyD:
				right = true;
				lastDirection = 1;
				break;
			case KeyDown:
			case KeyS:
				down = true;
				break;
			case KeyUp:
			case KeyW:
				up = true;
				break;
			case KeySpace:
				jump = true;
				break;
			case KeyControl:
				attack = true;
				break;
			default:
				break;
		}
	}
	
	void keyUp(KeyCode code) {
		switch (code) {
			case KeyLeft:
			case KeyA:
				left = false;
				break;
			case KeyRight:
			case KeyD:
				right = false;
				break;
			case KeyDown:
			case KeyS:
				down = false;
				break;
			case KeyUp:
			case KeyW:
				up = false;
				break;
			case KeySpace:
				jump = false;
				break;
			case KeyControl:
				attack = false;
				break;
			default:
				break;
		}
	}
}

int kore(int argc, char** argv) {
	Kore::System::init("LudumDare43", w * scale, h * scale);
	Kore::System::setCallback(update);
	
	loadNextLevel();
	
	cat_walk = new Animation();
	cat_walk->init("Tiles/cat_walking_anim.png", 5, Animation::AnimationTyp::Walking);
	playerWidth = cat_walk->getWidth();
	playerHeight = cat_walk->getHeight();
	cat_jump = new Animation();
	cat_jump->init("Tiles/cat_jumping_anim.png", 2, Animation::AnimationTyp::Jumping);
	cat_attack = new Animation();
	cat_attack->init("Tiles/cat_attack_anim.png", 2, Animation::AnimationTyp::Attacking);

	px = 0;
	py = tileHeight - playerHeight;
	playerCenter = vec3(px + playerWidth / 2, py + playerHeight / 2);
	
	guyPosition = vec2(0, 0);
	guy = new Animation();
	guy->init("Tiles/player.png", 9, Animation::AnimationTyp::Walking);
	
	g2 = new Graphics2::Graphics2(w, h, false);
	g2->setImageScaleQuality(Graphics2::Low);
	left = false;
	right = false;
	up = false;
	down = false;
	jump = false;
	attack = false;

	font14 = Kravur::load("Fonts/arial", FontStyle(), 14);
	font24 = Kravur::load("Fonts/arial", FontStyle(), 24);
	font34 = Kravur::load("Fonts/arial", FontStyle(), 34);
	font44 = Kravur::load("Fonts/arial", FontStyle(), 44);
	
	state = InGameState;
	
	Keyboard::the()->KeyDown = keyDown;
	Keyboard::the()->KeyUp = keyUp;
	


	Kore::System::start();

	return 0;
}
