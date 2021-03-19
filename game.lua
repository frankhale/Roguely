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
	Sprite_Info["spider_sprite_id"] = 4
	Sprite_Info["crab_sprite_id"] = 12
	Sprite_Info["bug_sprite_id"] = 17
	Sprite_Info["firewalker_sprite_id"] = 21
	Sprite_Info["crimsonshadow_sprite_id"] = 34
	Sprite_Info["purpleblob_sprite_id"] = 61
	Sprite_Info["orangeblob_sprite_id"] = 64
	Sprite_Info["golden_candle_sprite_id"] = 6

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

	-- Need to generate a map before we can add entities because the x,y
	-- of the entities is auto generated so that it won't collide with any
	-- entity
	generate_map("main", Game["map_width"], Game["map_height"])
	switch_map("main")

	Player_Pos = generate_random_point({ "common" })
	update_entity_position("common", "player", Player_Pos["x"], Player_Pos["y"])

	add_entities("coins", "item", {
		value_component = {
			value = 25
		}
	}, 100)

	add_entities("spiders", "enemy", {
		health_component = { health = 20 },
		stats_component = { attack = 1 }
	}, 100)

	add_entities("crabs", "enemy", {
		health_component = { health = 30 },
		stats_component = { attack = 2 }
	}, 80)

	add_entities("bugs", "enemy", {
		health_component = { health = 50 },
		stats_component = { attack = 2 }
	}, 80)

	add_entities("firewalkers", "enemy", {
		health_component = { health = 75 },
		stats_component = { attack = 4 }
	}, 50)

	add_entities("crimsonshadows", "enemy", {
		health_component = { health = 85 },
		stats_component = { attack = 5 }
	}, 40)

	add_entities("purpleblobs", "enemy", {
		health_component = { health = 95 },
		stats_component = { attack = 6 }
	}, 30)

	add_entities("orangeblobs", "enemy", {
		health_component = { health = 100 },
		stats_component = { attack = 7 }
	}, 20)

	add_entities("goldencandle", "item", {
		value_component = { value = 25000 },
		lua_component = {
			name = "win_component",
			type = "flags",
			properties = { win = true }
		}
	}, 1)

	Coins = get_entity_group_points("coins")
	Spiders = get_entity_group_points("spiders")
	Crabs = get_entity_group_points("crabs")
	Bugs =  get_entity_group_points("bugs")
	FireWalkers = get_entity_group_points("firewalkers")
	CrimsonShadows =  get_entity_group_points("crimsonshadows")
	PurpleBlobs = get_entity_group_points("purpleblobs")
	OrangeBlobs = get_entity_group_points("orangeblobs")
	GoldenCandle = get_entity_group_points("goldencandle")

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

function render_mini_map()
	for r = 1, Game["map_height"] do
		for c = 1, Game["map_width"] do
			local dx = (c - 1) + (Game["window_width"] - 150)
			local dy = (r - 1) + 10

			if (Game_Map[r][c] == 0) then
				set_draw_color(255,255,255,128)
				draw_point(dx, dy)
			elseif (Game_Map[r][c] == 9) then
				set_draw_color(0,0,0,128)
				draw_point(dx, dy)
			end

			if (dx == Player_Pos["x"] + (Game["window_width"] - 150) and dy == Player_Pos["y"] + 10) then
				set_draw_color(255,0,0,255)
				draw_filled_rect(dx - 3, dy - 3, 6, 6)
			end
		end
	end

	set_draw_color(0,0,0,255)
end

function _render(delta_time)
	for r = 1, get_view_port_height() do
		for c = 1, get_view_port_width() do
			local dx = ((c-1) * 32) - (get_view_port_x() * 32)
			local dy = ((r-1) * 32) - (get_view_port_y() * 32)

			if(Game_Light_Map[r][c] == 2) then
				draw_sprite("game-sprites", Game_Map[r][c], dx, dy)

				for rpk, rpv in pairs(Coins) do
					if(rpv["x"] == (c-1) and rpv["y"] == (r-1)) then
						draw_sprite("game-sprites", Sprite_Info["coin_sprite_id"], dx, dy)
					end
				end

				for epk, epv in pairs(Spiders) do
					if(epv["x"] == (c-1) and epv["y"] == (r-1)) then
						draw_sprite("game-sprites", Sprite_Info["spider_sprite_id"], dx, dy)
					end
				end

				for epk, epv in pairs(Crabs) do
					if(epv["x"] == (c-1) and epv["y"] == (r-1)) then
						draw_sprite("game-sprites", Sprite_Info["crab_sprite_id"], dx, dy)
					end
				end

				for epk, epv in pairs(Bugs) do
					if(epv["x"] == (c-1) and epv["y"] == (r-1)) then
						draw_sprite("game-sprites", Sprite_Info["bug_sprite_id"], dx, dy)
					end
				end

				for epk, epv in pairs(FireWalkers) do
					if(epv["x"] == (c-1) and epv["y"] == (r-1)) then
						draw_sprite("game-sprites", Sprite_Info["firewalker_sprite_id"], dx, dy)
					end
				end

				for epk, epv in pairs(CrimsonShadows) do
					if(epv["x"] == (c-1) and epv["y"] == (r-1)) then
						draw_sprite("game-sprites", Sprite_Info["crimsonshadow_sprite_id"], dx, dy)
					end
				end

				for epk, epv in pairs(PurpleBlobs) do
					if(epv["x"] == (c-1) and epv["y"] == (r-1)) then
						draw_sprite("game-sprites", Sprite_Info["purpleblob_sprite_id"], dx, dy)
					end
				end

				for epk, epv in pairs(OrangeBlobs) do
					if(epv["x"] == (c-1) and epv["y"] == (r-1)) then
						draw_sprite("game-sprites", Sprite_Info["orangeblob_sprite_id"], dx, dy)
					end
				end

				for epk, epv in pairs(GoldenCandle) do
					if(epv["x"] == (c-1) and epv["y"] == (r-1)) then
						draw_sprite("game-sprites", Sprite_Info["goldencandle_sprite_id"], dx, dy)
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

	render_mini_map()
end

function _tick(delta_time)
end

function _error(err)
	print("An error occurred: " .. err)
end