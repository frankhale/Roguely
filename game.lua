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
	Sprite_Info["coin_sprite_id"] = 14
	Sprite_Info["spider_sprite_id"] = 4
	Sprite_Info["crab_sprite_id"] = 12
	Sprite_Info["bug_sprite_id"] = 17
	Sprite_Info["firewalker_sprite_id"] = 21
	Sprite_Info["crimsonshadow_sprite_id"] = 34
	Sprite_Info["purpleblob_sprite_id"] = 61
	Sprite_Info["orangeblob_sprite_id"] = 64
	Sprite_Info["golden_candle_sprite_id"] = 6
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

	-- Need to generate a map before we can add entities because the x,y
	-- of the entities is auto generated so that it won't collide with any
	-- entity
	generate_map("main", Game["map_width"], Game["map_height"])
	switch_map("main")

	Player_Pos = generate_random_point({ "common" })
	update_entity_position("common", "player", Player_Pos["x"], Player_Pos["y"])

	Coins = add_entities("coins", "item", {
		value_component = {
			value = 25
		}
	}, 100)

	Spiders = add_entities("spiders", "enemy", {
		health_component = { health = 20 },
		stats_component = { attack = 1 }
	}, 100)

	Crabs = add_entities("crabs", "enemy", {
		health_component = { health = 30 },
		stats_component = { attack = 2 }
	}, 80)

	Bugs = add_entities("bugs", "enemy", {
		health_component = { health = 50 },
		stats_component = { attack = 2 }
	}, 80)

	FireWalkers = add_entities("firewalkers", "enemy", {
		health_component = { health = 75 },
		stats_component = { attack = 4 }
	}, 50)

	CrimsonShadows = add_entities("crimsonshadows", "enemy", {
		health_component = { health = 85 },
		stats_component = { attack = 5 }
	}, 40)

	PurpleBlobs = add_entities("purpleblobs", "enemy", {
		health_component = { health = 95 },
		stats_component = { attack = 6 }
	}, 30)

	OrangeBlobs = add_entities("orangeblobs", "enemy", {
		health_component = { health = 100 },
		stats_component = { attack = 7 }
	}, 20)

	GoldenCandle = add_entities("goldencandle", "item", {
		value_component = { value = 25000 },
		lua_component = {
			name = "win_component",
			type = "flags",
			properties = { win = true }
		}
	}, 1)

	-- for k,v in pairs(Coins) do
	-- 	--print(Coins[k]["item"]["point"]["x"], Coins[k]["item"]["point"]["y"])
	-- 	print(v["item"]["point"]["x"], v["item"]["point"]["y"])
	-- end

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
			Player = data["player"]
			Player_id = data["player"][Player_Id]
			Player_Pos["x"] = data["player"]["point"]["x"]
			Player_Pos["y"] = data["player"]["point"]["y"]

			-- TODO: we can check here to see if our position is the same as a coin or other pickup

			fov() -- recalculate FOV
		else
			-- Other entities were updated
		end
	elseif (event == "light_map") then
		Game_Light_Map = get_map("main", true)
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

function render_entity(entity_group, entity_type, sprite_sheet_name, entity_sprite_id_key, cx, cy, dx, dy)
	for epk, epv in pairs(entity_group) do
		if(epv[entity_type]["point"]["x"] == (cx-1) and epv[entity_type]["point"]["y"] == (cy-1)) then
			draw_sprite(sprite_sheet_name, Sprite_Info[entity_sprite_id_key], dx, dy)
		end
	end
end

function _render(delta_time)
	for r = 1, get_view_port_height() do
		for c = 1, get_view_port_width() do
			local dx = ((c-1) * Game_Sprites_Info.width) - (get_view_port_x() * Game_Sprites_Info.width)
			local dy = ((r-1) * Game_Sprites_Info.height) - (get_view_port_y() * Game_Sprites_Info.height)

			if(Game_Light_Map[r][c] == 2) then
				draw_sprite("game-sprites", Game_Map[r][c], dx, dy)

				render_entity(Coins, "item", "game-sprites", "coin_sprite_id", c, r, dx, dy)
				render_entity(GoldenCandle, "item", "game-sprites", "goldencandle_sprite_id", c, r, dx, dy)
				render_entity(Spiders, "enemy", "game-sprites", "spider_sprite_id", c, r, dx, dy)
				render_entity(Crabs, "enemy", "game-sprites", "crab_sprite_id", c, r, dx, dy)
				render_entity(Bugs, "enemy", "game-sprites", "bug_sprite_id", c, r, dx, dy)
				render_entity(FireWalkers, "enemy", "game-sprites", "firewalker_sprite_id", c, r, dx, dy)
				render_entity(CrimsonShadows, "enemy", "game-sprites", "crimsonshadow_sprite_id", c, r, dx, dy)
				render_entity(PurpleBlobs, "enemy", "game-sprites", "purpleblob_sprite_id", c, r, dx, dy)
				render_entity(OrangeBlobs, "enemy", "game-sprites", "orangeblob_sprite_id", c, r, dx, dy)

				if(Player_Pos["x"] == (c-1) and Player_Pos["y"] == (r-1)) then
					draw_sprite("game-sprites", Sprite_Info["player_sprite_id"], dx, dy)
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