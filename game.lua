Game = {
	window_title = "Roguely - A simple Roguelike in C++/Lua/SDL2",
	window_icon_path = "assets/icon.png",
	window_width = 1280,
	window_height = 768,
	map_width = 125,
	map_height = 125,
	view_port_width = 40,
	view_port_height = 24,
	music = false,
	soundtrack_path = "assets/ExitExitProper.mp3",
	font_path = "assets/VT323-Regular.ttf",
	logo_path = "assets/roguely-logo.png",
	start_game_image_path = "assets/press-space-bar-to-play.png",
	credit_image_path = "assets/credits.png",
	sounds = {
		coin = "assets/sounds/coin.wav",
		bump = "assets/sounds/bump.wav",
		combat = "assets/sounds/combat.wav",
		death = "assets/sounds/death.wav",
		pickup = "assets/sounds/pickup.wav"
	},
	dead = false,
	game_started = false
}

Game_Sprites_Info = {
	width = 32,
	height = 32
}

Player_Pos = {
	-- These is notional because after we generate a map we'll get a randomized
	-- position to start that is a known good ground tile
	x = 10,
	y = 10
}

function _init()
	Sprite_Info = add_sprite_sheet("game-sprites", "assets/roguelike.png", Game_Sprites_Info.width, Game_Sprites_Info.height)

	Sprite_Info["player_sprite_id"] = 3
	Sprite_Info["hidden_sprite_id"] = 18
	Sprite_Info["heart_sprite_id"] = 48

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

	-- Need to generate a map before we can call add_entities because the x,y
	-- of the entities is auto generated so that it won't collide with any
	-- entity
	generate_map("main", Game["map_width"], Game["map_height"])
	switch_map("main")

	local pos = generate_random_point({ "common" })
	update_entity_position("common", "player", pos["x"], pos["y"])

	add_entities("rewards", "item", {
		sprite_component = {
			name = "coin",
			spritesheet_name = "game-sprites",
			sprite_id = 14
		},
		value_component = {
			value = 25
		}
	}, 100)

	Items = add_entities("rewards", "item", {
		sprite_component = {
			name = "goldencandle",
			spritesheet_name = "game-sprites",
			sprite_id = 6
		},
		value_component = { value = 25000 },
		lua_component = {
			name = "win_component",
			type = "flags",
			properties = { win = true }
		}
	}, 1)

	add_entities("enemies", "enemy", {
		sprite_component = {
			name = "spider",
			spritesheet_name = "game-sprites",
			sprite_id = 4
		},
		health_component = { health = 20 },
		stats_component = { attack = 1 }
	}, 100)

	add_entities("enemies", "enemy", {
		sprite_component = {
			name = "crab",
			spritesheet_name = "game-sprites",
			sprite_id = 12
		},
		health_component = { health = 30 },
		stats_component = { attack = 2 }
	}, 50)

	add_entities("enemies", "enemy", {
		sprite_component = {
			name = "bug",
			spritesheet_name = "game-sprites",
			sprite_id = 17
		},
		health_component = { health = 50 },
		stats_component = { attack = 2 }
	}, 40)

	add_entities("enemies", "enemy", {
		sprite_component = {
			name = "firewalker",
			spritesheet_name = "game-sprites",
			sprite_id = 21
		},
		health_component = { health = 75 },
		stats_component = { attack = 4 }
	}, 30)

	add_entities("enemies", "enemy", {
		sprite_component = {
			name = "crimsonshadow",
			spritesheet_name = "game-sprites",
			sprite_id = 34
		},
		health_component = { health = 85 },
		stats_component = { attack = 5 }
	}, 25)

	add_entities("enemies", "enemy", {
		sprite_component = {
			name = "purpleblob",
			spritesheet_name = "game-sprites",
			sprite_id = 61
		},
		health_component = { health = 95 },
		stats_component = { attack = 6 }
	}, 20)

	Enemies = add_entities("enemies", "enemy", {
		sprite_component = {
			name = "orangeblob",
			spritesheet_name = "game-sprites",
			sprite_id = 64
		},
		health_component = { health = 100 },
		stats_component = { attack = 7 }
	}, 10)

	Game_Map = get_map("main", false)
	fov("main")
end

function _update(event, data)
	if(event == "key_event") then
		if data["key"] == "up" then
			if(is_tile_walkable(Player_Pos["x"], Player_Pos["y"], "up", "player", { "common" })) then
				update_entity_position("common", "player", Player_Pos["x"], Player_Pos["y"] - 1)
			else
				play_sound("bump")
			end
		 elseif data["key"] == "down" then
			if(is_tile_walkable(Player_Pos["x"], Player_Pos["y"], "down", "player", { "common" })) then
				update_entity_position("common", "player", Player_Pos["x"], Player_Pos["y"] + 1)
			else
				play_sound("bump")
			end
		 elseif data["key"] == "left" then
			if(is_tile_walkable(Player_Pos["x"], Player_Pos["y"], "left", "player", { "common" })) then
				update_entity_position("common", "player", Player_Pos["x"] - 1, Player_Pos["y"])
			else
				play_sound("bump")
			end
		 elseif data["key"] == "right" then
			if(is_tile_walkable(Player_Pos["x"], Player_Pos["y"], "right", "player", { "common" })) then
				update_entity_position("common", "player", Player_Pos["x"] + 1, Player_Pos["y"])
			else
				play_sound("bump")
			end
		 elseif data["key"] == "space" then
			-- warp player (for testing purposes)
			local pos = generate_random_point({ "common" })
			update_entity_position("common", "player", pos["x"], pos["y"])
		end

		if (Items[XY_Id] and Items[XY_Id]["item"]["components"]["sprite_component"]["name"] == "coin") then
			play_sound("coin")
		 	set_component_value("common", "player", "score_component", "score",
		 		Player["components"]["score_component"]["score"] +
		 		Items[XY_Id]["item"]["components"]["value_component"]["value"])
		 	remove_entity("rewards", Items[XY_Id]["item"]["id"])
		end

	elseif (event == "entity_event") then
		if (data["player"] ~= nil) then
			Player = data["player"]
			Player_id = data["player"][Player_Id]
			Player_Pos["x"] = data["player"]["point"]["x"]
			Player_Pos["y"] = data["player"]["point"]["y"]
			XY_Id = tostring(Player_Pos["x"] .. "_" .. Player_Pos["y"])
			fov("main")
		else
			if(data["entity_group_name"] == "rewards") then
			 	Items = data["entity_group"]
			end
		end
	elseif (event == "light_map") then
	 	Game_Light_Map = data
	end
end

function calculate_health_bar_width(health, starting_health, health_bar_max_width)
	local hw = health_bar_max_width;

	if (health < starting_health) then
		hw = ((health * (100 / starting_health)) * health_bar_max_width) / 100
	end

	return hw
end

function render_info_bar()
	set_draw_color(28 , 28, 28, 128)
	draw_filled_rect(10, 10, 290, 150)

	draw_sprite_scaled("game-sprites", Sprite_Info["heart_sprite_id"], 20, 20, Game_Sprites_Info.width * 2, Game_Sprites_Info.height * 2)

	local p_hw = calculate_health_bar_width(Player["components"]["health_component"]["health"], Player["components"]["health_component"]["max_health"], 200)

	set_draw_color(33, 33, 33, 255)
	draw_filled_rect((Game_Sprites_Info.width * 2 + 20), 36, 200, 24)

	if (Player["components"]["health_component"]["health"] <= Player["components"]["health_component"]["max_health"] / 3) then
		set_draw_color(255, 0, 0, 255) -- red player's health is in trouble
	else
		set_draw_color(8, 138, 41, 255) -- green for player
	end

	draw_filled_rect((Game_Sprites_Info.width * 2 + 20), 36, p_hw, 24)

	draw_text(tostring(Player["components"]["health_component"]["health"]), "medium", (Game_Sprites_Info.width * 3 + 70), 28)
	draw_text(tostring(Player["components"]["score_component"]["score"]), "large", 40, 90)

	set_draw_color(0, 0, 0, 255)
end

function render_mini_map()
	local offset_x = Game["window_width"] - 150
	local offset_y = 10
	for r = 1, Game["map_height"] do
		for c = 1, Game["map_width"] do
			local dx = (c - 1) + offset_x
			local dy = (r - 1) + offset_y

			if (Game_Map[r][c] == 0) then
				set_draw_color(255, 255, 255, 128)
				draw_point(dx, dy)
			elseif (Game_Map[r][c] == 9) then
				set_draw_color(0, 0, 0, 128)
				draw_point(dx, dy)
			end

			-- if (dx == gc_x + offset_x and dy == gc_y) then
			--  	set_draw_color(0, 255, 0, 255)
			--  	draw_filled_rect(dx - 3, dy - 3, 6, 6)
			-- end

			if (dx == Player_Pos["x"] + offset_x and dy == Player_Pos["y"] + offset_y) then
				set_draw_color(255, 0, 0, 255)
				draw_filled_rect(dx - 3, dy - 3, 6, 6)
			end
		end
	end

	set_draw_color(0,0,0,255)
end

function render_entity(entity_group, entity_type, p_id, dx, dy)
	if(entity_group[p_id] ~= nil) then
		local sprite_id = entity_group[p_id][entity_type]["components"]["sprite_component"]["sprite_id"]
		local spritesheet_name = entity_group[p_id][entity_type]["components"]["sprite_component"]["spritesheet_name"]
		draw_sprite(spritesheet_name, sprite_id, dx, dy)
	end
end

function _render(delta_time)
	for r = 1, get_view_port_height() do
		for c = 1, get_view_port_width() do
			local p_id = tostring((c-1) .. "_" .. (r-1))
			local dx = ((c-1) * Game_Sprites_Info.width) - (get_view_port_x() * Game_Sprites_Info.width)
			local dy = ((r-1) * Game_Sprites_Info.height) - (get_view_port_y() * Game_Sprites_Info.height)

			if(Game_Light_Map[r][c] == 2) then
				draw_sprite("game-sprites", Game_Map[r][c], dx, dy)

				render_entity(Items, "item", p_id, dx, dy)
				render_entity(Enemies, "enemy", p_id, dx, dy)

				if(Player_Pos["x"] == (c-1) and Player_Pos["y"] == (r-1)) then
					local p_hw = calculate_health_bar_width(Player["components"]["health_component"]["health"], Player["components"]["health_component"]["max_health"], 32)

					set_draw_color(8, 138, 41, 255)
					draw_filled_rect(dx, dy - 8, p_hw, 6)
					draw_sprite("game-sprites", Sprite_Info["player_sprite_id"], dx, dy)
					set_draw_color(0, 0, 0, 255)
				end
			else
				draw_sprite("game-sprites", Sprite_Info["hidden_sprite_id"] , dx, dy)
			end
		end
	end

	render_info_bar()
	render_mini_map()
end

function _tick(delta_time)
end

function _error(err)
	print("An error occurred: " .. err)
end