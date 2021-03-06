#include "Scoreboard.h"

Score_Row::Score_Row() {}

Score_Row::Score_Row(Score_Entry entry)
{
	player1_name = UI_Element();
	player1_score = UI_Element();
	colon = UI_Element();
	player2_name = UI_Element();
	player2_score = UI_Element();

	player1_name.text = entry.player1_name;
	player1_score.text = std::to_string(entry.player1_score);
	player2_name.text = entry.player2_name;
	player2_score.text = std::to_string(entry.player2_score);
	colon.text = ":";
}

void Score_Row::Init_Textures(SDL_Renderer ** renderer, TTF_Font * font, SDL_Surface *surface, SDL_Color &color)
{
	player1_name.Set_Texture(renderer, font, surface, color);
	player1_score.Set_Texture(renderer, font, surface, color);
	player2_name.Set_Texture(renderer, font, surface, color);
	player2_score.Set_Texture(renderer, font, surface, color);
	colon.Set_Texture(renderer, font, surface, color);
}

void Score_Row::Render(SDL_Renderer ** renderer)
{
	player1_name.Render(renderer);
	player1_score.Render(renderer, true);
	player2_name.Render(renderer);
	player2_score.Render(renderer, true);
	colon.Render(renderer);
}

void Score_Row::Free_Data()
{
	player1_name.Free_Data();
	player1_score.Free_Data();
	player2_name.Free_Data();
	player2_score.Free_Data();
	colon.Free_Data();
}

void Score_Row::UpdatePosition(int index)
{
	int y_pos = 150 + (index * 55);

	player1_name.rect = { 160, y_pos, 150, 50 };
	player1_score.rect = { player1_name.rect.w + player1_name.rect.x + 5, y_pos, 50, 50 };
	colon.rect = { player1_score.rect.x + player1_score.rect.w + 5, y_pos, 25, 50 };
	player2_name.rect = { colon.rect.x + colon.rect.w + 5, y_pos, 150, 50 };
	player2_score.rect = { player2_name.rect.x + player2_name.rect.w + 5, y_pos, 50, 50 };
}

std::string Score_Row::Convert_To_String(int number)
{
	std::string number_text = std::to_string(number);

	if (number < 10)
	{
		std::string space = " ";
		number_text.append(space);
	}

	return number_text;
}