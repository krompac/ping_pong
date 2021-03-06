#include <iostream>
#include <cstring>
#include <ctime>
#include <cstdlib>
#include <cmath>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <string>
#include <math.h>
#include <fstream>
#include "game.h"
#include "menu.h"
#include "ChangeTextureColor.h"

using namespace std;

Game::Game()
{
	this->window = nullptr;
	this->renderer = nullptr;
	this->surface = nullptr;
	this->left_score = nullptr;
	this->right_score = nullptr;
	this->colon_texture = nullptr;
	this->message_texture = nullptr;
	this->answers_textures[0] = nullptr;
	this->answers_textures[1] = nullptr;
	this->ball_picture = nullptr;
	this->winner_texture = nullptr;
	this->user_text_texture = nullptr;

	game_window = { 20, 20, 720, 480 };
	first_number = { 310, 520, 60, 70 };
	second_number = { 410, 520, 60, 70 };
	colon = { 375, 520, 30, 70 };
	
	user_text_rect = { 290, 225, 0, 80 };
	winner_rect = { 180, 70, 400, 100 };
	message_box = { 180, 190, 400, 150 };
	message = { message_box.x + 10, message_box.y + 10, 380, 60 };
	answers[0] = { 250, 280, 100, 50 };
	answers[1] = { 430, 280, 100, 50 };
	max_score = 1;

	menuitems = new SDL_Rect[4];

	menu = Menu(menuitems);

	speed_options = SpeedOptions(starting_ball_speed);
	score_options = ScoreOptions(max_score);
	ball_color = TextureColorOptions(&ball_picture, "Ball color", 190, Red);
	left_pad_options = PadColorOptions("Pad color", 270, left_pad_color, true);
	right_pad_options = PadColorOptions("Pad color", 350, right_pad_color);
	settings_window = Settings(menu, speed_options, score_options, ball_color, left_pad_options, right_pad_options);
	scoreboard_window = Scoreboard(menu);

	Initialize_Game_Components();
	pad_collision_surface = right_pad.h / 2;
	coefficient = pad_collision_surface / 10; //+ 0.5;

	color = { 255, 255, 255 };
	left_pad_color = Blue;
	right_pad_color = Blue;

	Init();
}

Game::~Game()
{
	delete menuitems;
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	TTF_Quit();
}

void Game::Initialize_Game_Components()
{
	//starting_ball_speed = 5;
	frame_time = 0;
	speed_x = starting_ball_speed;
	speed_y = 0;

	left_pad = { 20, 20, 20, 100 };
	right_pad = { 720, 20, 20, 100 };
	ball = { 370, 250, 20, 20 };

	first_score = 0;
	second_score = 0;
	text_to_convert = to_string(first_score);
	
	player1_entered_name = player2_entered_name = false;
	ball_going_right = ball_going_up = true;
	pause = false;
	game_won = false;

	user_text = "";
}

void Game::Free_Texture(SDL_Texture *texture)
{
	if (texture != nullptr)
	{
		SDL_DestroyTexture(texture);
		texture = nullptr;
	}

	if (surface != nullptr)
	{
		SDL_FreeSurface(surface);
		surface = nullptr;
	}
}

void Game::Entry_Input(char *player_name, int &player_score, int achieved_score, bool &player_entered_name)
{
	string space = " ";
	while (user_text.length() < 10)
	{
		user_text.append(space.c_str());
	}

	SDL_strlcpy(player_name, user_text.c_str(), 10);
	player_score = achieved_score;
	player_entered_name = true;

	user_text = "";
	user_text_rect.w = 0;
}

void Game::Free()
{
}

void Game::Texture_From_Text(SDL_Texture **texture, int number, string text)
{
	Free_Texture(*texture);
	if (text != "")
	{
		text_to_convert = text;
	}
	else
	{
		text_to_convert = to_string(number);
	}
	
	surface = TTF_RenderText_Solid(font, text_to_convert.c_str(), color);
	*texture = SDL_CreateTextureFromSurface(renderer, surface);
}

void Game::Render_Game_Window(bool is_message)
{
	SDL_RenderClear(renderer);
	
	Fill_Rect_With_Color(renderer, left_pad, left_pad_color);
	Fill_Rect_With_Color(renderer, right_pad, right_pad_color);

	SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0xff, 0xff);

	SDL_RenderDrawRect(renderer, &first_number);
	SDL_RenderDrawRect(renderer, &second_number);
	SDL_RenderDrawRect(renderer, &colon);

	menu.Render_Menu(renderer);

	SDL_SetRenderDrawColor(renderer, 0xCA, 0xCE, 0xAD, 0xff);
	SDL_RenderDrawRect(renderer, &game_window);

	if (pause)
	{
		SDL_RenderDrawRect(renderer, &menuitems[menu_position]);
	}

	SDL_RenderCopy(renderer, colon_texture, NULL, &colon);
	SDL_RenderCopy(renderer, left_score, NULL, &first_number);
	SDL_RenderCopy(renderer, right_score, NULL, &second_number);
	SDL_RenderCopy(renderer, ball_picture, NULL, &ball);

	if (is_message)
	{
		SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xff);

		SDL_RenderFillRect(renderer, &message_box);
		SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0xff, 0xff);
		SDL_RenderDrawRect(renderer, &message_box);

		SDL_RenderCopy(renderer, message_texture, NULL, &message);

		SDL_SetRenderDrawColor(renderer, 0xCA, 0xCE, 0xAD, 0xff);
		SDL_RenderDrawRect(renderer, &answers[message_position]);
		SDL_RenderCopy(renderer, answers_textures[0], NULL, &answers[0]);
		SDL_RenderCopy(renderer, answers_textures[1], NULL, &answers[1]);
	}

	if (game_won)
	{
		SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0xff, 0xff);
		SDL_RenderDrawRect(renderer, &winner_rect);
		SDL_RenderCopy(renderer, winner_texture, NULL, &winner_rect);

		if (!player2_entered_name)
		{
			SDL_RenderDrawRect(renderer, &user_text_rect);

			if (!user_text.empty())
			{
				SDL_RenderCopy(renderer, user_text_texture, NULL, &user_text_rect);
			}
		}
	}
	
	SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xff);

	SDL_RenderPresent(renderer);
}

void Game::Initialize_Message(string message)
{
	surface = TTF_RenderText_Solid(font, message.c_str(), color);
	message_texture = SDL_CreateTextureFromSurface(renderer, surface);
	surface = TTF_RenderText_Solid(font, "Yes", color);
	answers_textures[0] = SDL_CreateTextureFromSurface(renderer, surface);
	surface = TTF_RenderText_Solid(font, "No", color);
	answers_textures[1] = SDL_CreateTextureFromSurface(renderer, surface);
}

void Game::Restart_Game()
{
	while (Text_Input());
	while (Text_Input());

	Initialize_Message("Would you like to start again?");
	while (Message_Box_Action());
}

bool Game::Message_Box_Action()
{
	Render_Game_Window(true);

	while (SDL_PollEvent(&message_event))
	{
		if (message_event.type == SDL_QUIT)
			this->~Game();
		else if (message_event.type == SDL_KEYDOWN)
		{
			if (message_event.key.keysym.sym == SDLK_LEFT)
			{
				if (message_position)
				{
					message_position--;
				}
			}
			else if (message_event.key.keysym.sym == SDLK_RIGHT)
			{
				if (!message_position)
				{
					message_position++;
				}
			}
			else if (message_event.key.keysym.sym == SDLK_RETURN || message_event.key.keysym.sym == SDLK_SPACE)
			{
				if (!message_position)
				{
					Initialize_Game_Components();
					Texture_From_Text(&left_score, 0);
					Texture_From_Text(&right_score, 0);
				}

				if (!game_won)
				{
					pause = false;
					Render_Game_Window();
					SDL_Delay(500);
				}

				return false;
			}
			
			Render_Game_Window(true);
			SDL_Delay(1);

			return true;
		}
	}

	Render_Game_Window(true);
	SDL_Delay(1);

	return true;
}

void Game::Update_Ball_Origin()
{
	ball_origin = { ball.x + ball.w / 2, ball.y + ball.h / 2 };
}

void Game::Ball_Movement()
{
	if (!pause)
	{
		int y = ball.y + ball.h / 2;

		if (frame_time + 1000 / 60 < SDL_GetTicks())
		{
			frame_time = SDL_GetTicks();
			if (ball_going_right)
			{
				ball.x += speed_x;
				Update_Ball_Origin();

				if (ball_origin.x + ball.w / 2 >= game_window.w + game_window.x)
				{
					ball.x = game_window.x + game_window.w - ball.w;
					Update_Ball_Origin();
					ball_going_right = false;
					first_score++;
					Texture_From_Text(&left_score, first_score);
				}

				if (Check_Corner() || ((ball_origin.x + ball.w / 2 >= right_pad.x) && (ball_origin.y - ball.h / 2) >= right_pad.y && ball_origin.y + ball.h / 2  <= (right_pad.y + right_pad.h)))
				{
					ball_going_right = false;
					z_right_pad = right_pad.y + (right_pad.h / 2);
					speed_y = abs(z_right_pad - ball_origin.y) / coefficient;
					speed_x = starting_ball_speed + 1 - (speed_y / starting_ball_speed);
					ball_going_up = (z_right_pad < y);
					starting_ball_speed += 0.2;
				}
			}
			else
			{
				ball.x -= speed_x;
				Update_Ball_Origin();

				if (ball.x <= game_window.x)
				{
					ball.x = game_window.x;
					Update_Ball_Origin();
					ball_going_right = true;
					second_score++;
					Texture_From_Text(&right_score, second_score);
				}

				if (Check_Corner() || ((ball_origin.x - ball.w / 2 <= left_pad.x + left_pad.w) && (ball_origin.y - ball.h / 2) >= left_pad.y && ball_origin.y + ball.h / 2 <= (left_pad.y + left_pad.h)))
				{
					ball_going_right = true;
					z_left_pad = left_pad.y + (left_pad.h / 2);
					speed_y = abs(z_left_pad - y) / coefficient;
					speed_x = starting_ball_speed + 1 - (speed_y / starting_ball_speed);
					ball_going_up = (z_left_pad < y);
					starting_ball_speed += 0.2;
				}
			}

			if (ball_going_up)
			{
				ball.y += speed_y;
				Update_Ball_Origin();

				if (ball.y + ball.h >= game_window.h + game_window.y)
				{
					ball_going_up = false;
					ball.y = game_window.h;
					Update_Ball_Origin();
				}
			}
			else
			{
				ball.y -= speed_y;
				Update_Ball_Origin();

				if (ball.y <= game_window.y)
				{
					ball_going_up = true;
					ball.y = game_window.y;
					Update_Ball_Origin();
				}
			}
		}
	}

	if (first_score == max_score)
	{
		Texture_From_Text(&winner_texture, 0, "Player 1 Wins");
		game_won = true;
		Restart_Game();
	}
	else if (second_score == max_score)
	{
		Texture_From_Text(&winner_texture, 0, "Player 2 Wins");
		game_won = true;
		Restart_Game();
	}
}

void Game::Enter_Record()
{
	if (!player1_entered_name)
	{
		Entry_Input(entry.player1_name, entry.player1_score, first_score, player1_entered_name);
	}
	else
	{
		Entry_Input(entry.player2_name, entry.player2_score, second_score, player2_entered_name);
		scoreboard_window.Add_Score_Entry(entry, true);
	}
}

bool Game::Track_Rightpad()
{
	const Uint8 *keyboard_state_array = SDL_GetKeyboardState(NULL);

	if (!pause)
	{
		if (keyboard_state_array[SDL_SCANCODE_UP])
		{
			if (right_pad.y - 1 >= game_window.y)
				right_pad.y -= 1;
		}
		else if (keyboard_state_array[SDL_SCANCODE_DOWN])
		{
			if (right_pad.y + 1 + right_pad.h <= game_window.h + game_window.y)
				right_pad.y += 1;
		}
	}
	return true;
}

bool Game::Main_Loop()
{
	const Uint8 *keyboard_state_array = SDL_GetKeyboardState(NULL);

	if (!pause)
	{
		if (keyboard_state_array[SDL_SCANCODE_W])
		{
			if (left_pad.y - 1 >= game_window.y)
				left_pad.y -= 1;
		}
		else if (keyboard_state_array[SDL_SCANCODE_S])
		{
			if (left_pad.y + 1 + left_pad.h <= game_window.h + game_window.y)
				left_pad.y += 1;
		}
	}

	while (SDL_PollEvent(&event_handler))
	{
		if (event_handler.type == SDL_QUIT)
			return false;
		else if (event_handler.type == SDL_KEYDOWN)
		{
			if (pause)
			{
				if (event_handler.key.keysym.sym == SDLK_UP || event_handler.key.keysym.sym == SDLK_w)
				{
					if (menu_position > 0)
					{
						menu_position--;
					}
				}
				else if (event_handler.key.keysym.sym == SDLK_DOWN || event_handler.key.keysym.sym == SDLK_s)
				{
					if (menu_position < 3)
					{
						menu_position++;
					}
				}
				else if (event_handler.key.keysym.sym == SDLK_SPACE || event_handler.key.keysym.sym == SDLK_RETURN)
				{
					if (menu_position == 0)
					{
						message_position = 0;
						Restart_Game();
					}
					else if (menu_position == 1)
					{
						scoreboard_window.Init_Texture(&renderer, font);
						do
						{
							scoreboard_window.Render(&renderer);
						} while (scoreboard_window.Window_Action(&renderer));
						scoreboard_window.Free_Data();
					}
					else if (menu_position == 2)
					{
						settings_window.Init_Textures(&renderer, font);
						do
						{
							settings_window.Render(&renderer);
						} while (settings_window.Window_Action(&renderer));
						settings_window.FreeData();
					}
					else if (menu_position == 3)
					{
						message_position = 1;
						Initialize_Message("Are you sure you want to guit the game?");
						while (Message_Box_Action());

						//vraca message_position jer je pozicija Yes odgovora na 0.-oj poziciji (false), a No je 1.-oj poziciji (true)
						return message_position;
					}
				}
			}

			if (event_handler.key.keysym.sym == SDLK_ESCAPE)
			{
				pause = !pause;
			}

			Ball_Movement();
			Render_Game_Window();
		}
	}

	Ball_Movement();
	Render_Game_Window();
	SDL_Delay(1);

	return true;
}

bool Game::Check_Corner()
{
	int x_coord = - ball_origin.x;
	int y_coord = - ball_origin.y;

	x_coord += (ball_going_right) ? right_pad.x : left_pad.x + left_pad.w;
	y_coord += (ball_going_right) ? right_pad.y : left_pad.y;
	
	return ((pow((x_coord), 2) + pow((y_coord), 2)) <= pow(ball.w / 2, 2)) || ((pow((x_coord), 2) + pow((y_coord + right_pad.h), 2)) <= pow(ball.w / 2, 2));
}

bool Game::Text_Input()
{
	while (SDL_PollEvent(&message_event))
	{
		if (message_event.type == SDL_QUIT)
			this->~Game();
		else if (message_event.type == SDL_KEYDOWN)
		{
			if (user_text.length() < 9)
			{
				switch (message_event.key.keysym.sym)
				{
					case SDLK_q:
						user_text += "q";
						break;
					case SDLK_w:
						user_text += "w";
						break;
					case SDLK_e:
						user_text += "e";
						break;
					case SDLK_r:
						user_text += "r";
						break;
					case SDLK_t:
						user_text += "t";
						break;
					case SDLK_z:
						user_text += "z";
						break;
					case SDLK_u:
						user_text += "u";
						break;
					case SDLK_i:
						user_text += "i";
						break;
					case SDLK_o:
						user_text += "o";
						break;
					case SDLK_p:
						user_text += "p";
						break;
					case SDLK_a:
						user_text += "a";
						break;
					case SDLK_s:
						user_text += "s";
						break;
					case SDLK_d:
						user_text += "d";
						break;
					case SDLK_f:
						user_text += "f";
						break;
					case SDLK_g:
						user_text += "g";
						break;
					case SDLK_h:
						user_text += "h";
						break;
					case SDLK_j:
						user_text += "j";
						break;
					case SDLK_k:
						user_text += "k";
						break;
					case SDLK_l:
						user_text += "l";
						break;
					case SDLK_y:
						user_text += "y";
						break;
					case SDLK_x:
						user_text += "x";
						break;
					case SDLK_c:
						user_text += "c";
						break;
					case SDLK_v:
						user_text += "v";
						break;
					case SDLK_b:
						user_text += "b";
						break;
					case SDLK_n:
						user_text += "n";
						break;
					case SDLK_m:
						user_text += "m";
						break;
					case SDLK_SPACE:
						user_text += " ";
						break;
					case SDLK_BACKSPACE:
						if (!user_text.empty())
						{
							if (user_text.back() == ' ')
							{
								user_text_rect.x += 5;
							}
							else
							{
								user_text_rect.x += 10;
							}

							user_text.pop_back();
						}
						break;
					case SDLK_RETURN:
						Enter_Record();
						return false;
				}
			}
			else
			{
				switch (message_event.key.keysym.sym)
				{
					case SDLK_BACKSPACE:
						user_text.pop_back();
						break;
					case SDLK_RETURN:
						Enter_Record();
						return false;
				}
			}

			Set_Text_Width();
			Texture_From_Text(&user_text_texture, 0, user_text);
		}
	}

	Render_Game_Window();
	SDL_Delay(1);

	return true;
}

void Game::Set_Text_Width()
{
	int lenght = 0;
	int x_pos = 290;

	for (int i = 0; i < user_text.length(); i++)
	{
		if (user_text[i] == ' ')
		{
			lenght += 30;
			x_pos -= 5;
			
		}
		else
		{
			lenght += 60;
			x_pos -= 20;
		}
	}

	user_text_rect.w = lenght;
	user_text_rect.x = x_pos;
}

void Game::Init()
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		cout << "SDL nije mogel inicijalizirati!\nError: " << SDL_GetError() << endl;
	}
	else
	{
		if (SDL_CreateWindowAndRenderer(WIDTH, HEIGHT, SDL_WINDOW_SHOWN, &window, &renderer))
		{
			cout << "Gre�ka kod pravljenja prozora i/ili renderera!\nError: " << SDL_GetError() << endl;
		}

		int imgFlags = IMG_INIT_PNG;
		if (!(IMG_Init(imgFlags) & imgFlags))
		{
			printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
		}

		if (TTF_Init() == -1)
		{
			cout << "Greska kod TTF inita!\n";
		}
		font = TTF_OpenFont("images/Sans.ttf", 100);
		
		menu.Init(surface, font, color, renderer);

		SDL_ShowCursor(SDL_DISABLE);

		surface = TTF_RenderText_Solid(font, ":", color);
		colon_texture = SDL_CreateTextureFromSurface(renderer, surface);
		Texture_From_Text(&left_score, 0);
		Texture_From_Text(&right_score, 0);
		Texture_From_Text(&user_text_texture, 0, user_text);

		surface = IMG_Load("images/circle.png");
		ball_picture = SDL_CreateTextureFromSurface(renderer, surface);
		ball_color.Init_Textures(&renderer, font);
		
		scoreboard_window.Init_Data();
		
		Render_Game_Window();

		while (Main_Loop() && Track_Rightpad() && !game_won);
	}
}