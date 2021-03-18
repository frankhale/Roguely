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
Test_Map = {}
Test_Score = -1

function _init()
	Sprite_Info = add_sprite_sheet("game-sprites", "assets/roguelike.png", 32, 32)

	add_entity("common", "player", 10, 10, {
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

	-- We have to generate a map or things will blow up for sure!
	generate_map("main", 100, 40)
	switch_map("main")

	--local w = is_tile_walkable(10, 10, "left", "player", { "common" })
	--print(w)

	local point = generate_random_point({ "common" })
	print("x = " .. point.x .. " | y = " .. point.y)

	Test_Map = get_test_map()
	--Test_Map = get_map("main")

	-- set_component_value("common", "player", "score_component", "score", 100);
	-- Test_Score = get_component_value("common", "player", "score_component", "score")

	-- for i, sr in pairs(sprite_info) do
	--	print(i .. " : x = " .. sprite_info[i].x .. " y = " .. sprite_info[i].y .. " w = " .. sprite_info[i].w .. " h = " .. sprite_info[i].h)
	-- end
end

function _update(event, data)
	-- play_sound("coin")
	-- play_sound("pickup")
	-- play_sound("combat")
	-- play_sound("death")

	if(event == "key_event") then
		if data["key"] == "up" then
			play_sound("coin")
		 elseif data["key"] == "down" then
			play_sound("pickup")
		 elseif data["key"] == "left" then
			play_sound("combat")
		 elseif data["key"] == "right" then
			play_sound("death")
		end
	elseif (event == "entity_event") then
		-- do something
	end
end

function _render(delta_time)
	draw_text("Hello world from Lua", "large", 10, 10)
	draw_text("C++ and Lua are a great match!!!", "medium", 10, 60)

	-- draw_text("Player Score: " .. Test_Score, "small", 500, 20)

	-- for r = 1, 100 do
	-- 	for c = 1, 40 do
	-- 		draw_sprite("game-sprites", Test_Map[r][c], (c - 1) * 32, (r - 1) * 32)
	-- 	end
	-- end

	for r = 1, 10 do
		for c = 1, 10 do
	 		draw_sprite("game-sprites", Test_Map[r][c], (c - 1) * 32 + 10, (r - 1) * 32 + 120)
	 	end
	 end

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