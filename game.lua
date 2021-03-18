Game = {
	window_title = "Roguely - A simple Roguelike in C++/Lua/SDL2",
	window_icon_path = "assets/icon.png",
	window_width = 1280,
	window_height = 768,
	map_width = 125,
	map_height = 125,
	view_port_width = 20,
	view_port_height = 12,
	music = false,
	soundtrack_path = "assets/ExitExitProper.mp3",
	font_path = "assets/VT323-Regular.ttf",
	logo_path = "assets/roguely-logo.png",
	start_game_image_path = "assets/press-space-bar-to-play.png",
	credit_image_path = "assets/credits.png",
	sounds = {
		coin = "assets/sounds/coin.wav",
		combat = "assets/sounds/combat.wav",
		death = "assets/sounds/death.wav",
		pickup = "assets/sounds/pickup.wav"
	}
}

Player_Pos = {
	x = 10,
	y = 10
}

function _init()
	Sprite_Info = add_sprite_sheet("game-sprites", "assets/roguelike.png", 32, 32)

	Sprite_Info["player_sprite_id"] = 3
	Sprite_Info["hidden_sprite_id"] = 18
	Sprite_Info["coin_sprite_id"] = 14

	Player_Id = add_entity("common", "player", Player_Pos["x"], Player_Pos["y"], {
		sprite_component = {
			name = "player",
			spritesheet_name = "game-sprites",
			sprite_id = 3
		},
		health_component = { health = 60 },
		stats_component = { attack = 1 },
		score_component = { score = 0 },
		inventory_component = {
			items = {
				health_potion = 3
			}
		}
	})

	generate_map("main", Game["map_width"], Game["map_height"])
	switch_map("main")

	Coin_Ids = add_entities("rewards", "item", {
		value_component = {
			value = "25"
		}
	}, 100)

	Rewards_Points = get_entity_group_points("rewards")

	Player_Pos = generate_random_point({ "common" })
	update_entity_position("common", "player", Player_Pos["x"], Player_Pos["y"])

	Game_Map = get_map("main", false)
	Game_Light_Map = get_map("main", true)
end

function _update(event, data)
	if(event == "key_event") then
		if data["key"] == "up" then
			if(is_tile_walkable(Player_Pos["x"], Player_Pos["y"], "up", "player", { "common" })) then
				update_entity_position("common", "player", Player_Pos["x"], Player_Pos["y"] - 1)
			end
		 elseif data["key"] == "down" then
			if(is_tile_walkable(Player_Pos["x"], Player_Pos["y"], "down", "player", { "common" })) then
				update_entity_position("common", "player", Player_Pos["x"], Player_Pos["y"] + 1)
			end
		 elseif data["key"] == "left" then
			if(is_tile_walkable(Player_Pos["x"], Player_Pos["y"], "left", "player", { "common" })) then
				update_entity_position("common", "player", Player_Pos["x"] - 1, Player_Pos["y"])
			end
		 elseif data["key"] == "right" then
			if(is_tile_walkable(Player_Pos["x"], Player_Pos["y"], "right", "player", { "common" })) then
				update_entity_position("common", "player", Player_Pos["x"] + 1, Player_Pos["y"])
			end
		end
	elseif (event == "entity_event") then
		if (data["player"] ~= nil) then
			Player_Pos["x"] = data["player"][Player_Id]["point"]["x"]
			Player_Pos["y"] = data["player"][Player_Id]["point"]["y"]
			fov()
		end
	elseif (event == "light_map") then
		Game_Light_Map = get_map("main", true)
	end
end

function _render(delta_time)
	for r = 1, get_view_port_height() do
		for c = 1, get_view_port_width() do
			local dx = ((c-1) * 32) - (get_view_port_x() * 32)
			local dy = ((r-1) * 32) - (get_view_port_y() * 32)

			if(Game_Light_Map[r][c] == 2) then
				draw_sprite("game-sprites", Game_Map[r][c], dx, dy)


				for rpk, rpv in pairs(Rewards_Points) do
					if(rpv["x"] == (c-1) and rpv["y"] == (r-1)) then
						draw_sprite("game-sprites", Sprite_Info["coin_sprite_id"], dx, dy)
					end
				end

				if(Player_Pos["x"] == (c-1) and Player_Pos["y"] == (r-1)) then
					draw_sprite("game-sprites", Sprite_Info["player_sprite_id"], dx, dy)
				end
			else
				draw_sprite("game-sprites", Sprite_Info["hidden_sprite_id"] , dx, dy)
			end
		end
	end
end

function _tick(delta_time)
end

function _error(err)
	print("An error occurred: " .. err)
end