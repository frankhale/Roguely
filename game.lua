Game = {
	window_title = "Roguely - A simple Roguelike in SDL2 and C++/Lua",
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

Entities = {}
Sprite_Info = {}
Game_Map = {}
Player_Id = ""
Player_Pos = {
	x = 10,
	y = 10
}

function print_view_port()
	local vh = get_view_port_height()
	local vw = get_view_port_width()
	local vx = get_view_port_x()
	local vy = get_view_port_y()

	print("vh = " .. vh)
	print("vw = " .. vw)
	print("vx = " .. vx)
	print("vy = " .. vy)
end

function _init()
	Sprite_Info = add_sprite_sheet("game-sprites", "assets/roguelike.png", 32, 32)

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

	print("Player Id: " .. Player_Id)

	-- add_entity("coins", "coin", 1, 1, {
	-- 	value_component = {
	-- 		value = 10
	-- 	}
	-- })

	-- add_entity("common", "item", 20, 20, {
	-- 	lua_component = {
	-- 		name = "bar",
	-- 		type = "xyz",
	-- 		props = {
	-- 			baz = "boz"
	-- 		}
	-- 	}
	-- })

	local sp_info = get_sprite_info("game-sprites")

	print("map_height = " .. Game["map_height"])
	print("map_width = " .. Game["map_width"])

	generate_map("main", Game["map_width"], Game["map_height"])
	switch_map("main")

	--local w = is_tile_walkable(10, 10, "left", "player", { "common" })
	--print(w)

	local point = generate_random_point({ "common" })
	Player_Pos["x"] = point["x"]
	Player_Pos["y"] = point["y"]

	print("x = " .. point.x .. " | y = " .. point.y)
	print("player x = " .. Player_Pos["x"] .. " | player y = " .. Player_Pos["y"])

	update_entity_position("common", "player", Player_Pos["x"], Player_Pos["y"])

	--Game_Map = get_test_map()
	Game_Map = get_map("main")

	-- set_component_value("common", "player", "score_component", "score", 100);
	-- Test_Score = get_component_value("common", "player", "score_component", "score")

	-- for i, sr in pairs(sprite_info) do
	--	print(i .. " : x = " .. sprite_info[i].x .. " y = " .. sprite_info[i].y .. " w = " .. sprite_info[i].w .. " h = " .. sprite_info[i].h)
	-- end
	print_view_port()
end

function _update(event, data)
	if(event == "key_event") then
		--play_sound("death")
		--play_sound("coin")
		--play_sound("pickup")
		--play_sound("combat")

		--print_view_port()

		if data["key"] == "up" then
			--set_component_value("common", "player", "score_component", "score", 100);
			update_entity_position("common", "player", Player_Pos["x"], Player_Pos["y"] - 1)
		 elseif data["key"] == "down" then
			update_entity_position("common", "player", Player_Pos["x"], Player_Pos["y"] + 1)
		 elseif data["key"] == "left" then
			update_entity_position("common", "player", Player_Pos["x"] - 1, Player_Pos["y"])
		 elseif data["key"] == "right" then
			update_entity_position("common", "player", Player_Pos["x"] + 1, Player_Pos["y"])
		end
	elseif (event == "entity_event") then

		-- local lengthNum = 0
		-- for k, v in pairs(data) do -- for every key in the table with a corresponding non-nil value
		-- lengthNum = lengthNum + 1
		-- end
		-- print("DATA length: " .. lengthNum)
		-- --["score_component"]["score"]

		--  for k, v in pairs(data) do -- for every key in the table with a corresponding non-nil value
		-- 	print("key :" .. k)
		-- 	print(data[k])

		-- 	for k1, v1 in pairs(data[k]) do -- for every key in the table with a corresponding non-nil value
		-- 		print("key1 :" .. k1)
		-- 		print(data[k][k1])

		-- 		for k2, v2 in pairs(data[k][k1]) do -- for every key in the table with a corresponding non-nil value
		-- 		 	print("key2 :" .. k2)
		-- 		 	print(data[k][k1][k2])
		-- 		 end
		-- 	end
		-- end

		if (data["player"] ~= nil) then
			Player_Pos["x"] = data["player"][Player_Id]["point"]["x"]
			Player_Pos["y"] = data["player"][Player_Id]["point"]["y"]

			-- print("LUA_UPDATE : player x = " .. Player_Pos["x"])
			-- print("LUA_UPDATE : player y = " .. Player_Pos["y"])
			--print("player score: " .. data["player"][Player_Id]["components"]["score_component"]["score"])
		end
	end
end

function _render(delta_time)
	for r = 1, get_view_port_height() do
		for c = 1, get_view_port_width() do
			local dx = ((c-1) * 32) - (get_view_port_x() * 32)
			local dy = ((r-1) * 32) - (get_view_port_y() * 32)

			draw_sprite("game-sprites", Game_Map[r][c], dx, dy)

			if(Player_Pos["x"] == c and Player_Pos["y"] == r) then
				draw_sprite("game-sprites", 3, dx, dy)
			end
		end
	end

	--draw_text("Hello world from Lua", "large", 10, 10)
	--draw_text("C++ and Lua are a great match!!!", "medium", 10, 60)
	-- draw_text("Player Score: " .. Test_Score, "small", 500, 20)
	-- for r = 1, 10 do
	-- 	for c = 1, 10 do
	--  		draw_sprite("game-sprites", Game_Map[r][c], (c - 1) * 32 + 10, (r - 1) * 32 + 120)
	--  	end
	-- end
	-- local x_counter = 0
	-- local y_counter = 0
	-- for i, sr in pairs(Sprite_Info) do
	-- 	print("Sprite index: " .. i)
	-- 	draw_sprite("game-sprites", i, sr.x, sr.y + 100)
	-- 	if x_counter < 16 then
	-- 		x_counter = x_counter + 1
	-- 	else
	-- 		y_counter = y_counter + 1
	-- 		x_counter = 0
	-- 	end
	-- end
end

function _tick(delta_time)
end

function _error(err)
	print("An error occurred: " .. err)
end