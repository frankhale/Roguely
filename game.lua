-- game.lua
--
-- MIT License
--
-- Copyright (c) 2021 Frank Hale
--
-- Permission is hereby granted, free of charge, to any person obtaining a copy
-- of this software and associated documentation files (the "Software"), to deal
-- in the Software without restriction, including without limitation the rights
-- to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
-- copies of the Software, and to permit persons to whom the Software is
-- furnished to do so, subject to the following conditions:
--
-- The above copyright notice and this permission notice shall be included in all
-- copies or substantial portions of the Software.
--
-- THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
-- IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
-- FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
-- AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
-- LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
-- OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
-- SOFTWARE.

Game = {
	window_title = "Roguely - A simple Roguelike in SDL2/C++/Lua",
	window_icon_path = "assets/icon.png",
	window_width = 1280,
	window_height = 768,
	map_width = 125,
	map_height = 125,
	view_port_width = 40,
	view_port_height = 24,
	spritesheet_path = "assets/roguelike.png",
	soundtrack_path = "assets/ExitExitProper.mp3",
	font_path = "assets/VT323-Regular.ttf",
	logo_image_path = "assets/roguely-logo.png",
	start_game_image_path = "assets/press-space-bar-to-play.png",
	credit_image_path = "assets/credits.png",
	sounds = {
		coin = "assets/sounds/coin.wav",
		bump = "assets/sounds/bump.wav",
		combat = "assets/sounds/combat.wav",
		death = "assets/sounds/death.wav",
		pickup = "assets/sounds/pickup.wav",
		warp = "assets/sounds/warp.wav",
		move = "assets/sounds/move.wav"
	},
	music = true,
	debug = true,
	dead = false,
	started = false,
	won = false,
	lost = false,
	total_enemies_killed = 0, -- TODO: This should be a component on the player
	-- This is here for easy access
	player_pos = {
		-- These is notional because after we generate a map we'll get a randomized
		-- position to start that is a known good ground tile
		x = 10,
		y = 10
	},
	-- used for quick reference even though entities have a sprite component
	sprite_info = {
		width = 32,
		height = 32,
		health_gem = 1,
		attack_gem = 2,
		treasure_chest = 13,
		attack_bonus_gem = 2,
		player_sprite_id = 3,
		hidden_sprite_id = 18,
		heart_sprite_id = 48,
		coin_sprite_id = 14,
		spider_sprite_id = 4,
		lurcher_sprite_id = 5,
		crab_sprite_id = 12,
		bug_sprite_id = 17,
		firewalker_sprite_id = 21,
		crimsonshadow_sprite_id = 34,
		purpleblob_sprite_id = 61,
		orangeblob_sprite_id = 64,
		mantis_sprite_id = 54,
		golden_candle_sprite_id = 6,
		wall_with_grass_1 = 35,
		wall_with_grass_2 = 36
	},
	health_recovery_timer = 0,
	preemptive_enemy_attack_timer = 0,
	level_check_timer = 0,
	logic_timer = 0,
	health_recovery = 10,
	action_log = {},
	entities = {
		player = {
			components = {
				sprite_component = {
					name = "player",
					spritesheet_name = "game-sprites",
					sprite_id = 3
				},
				health_component = { health = 100 },
				stats_component = { attack = 1 },
				score_component = { score = 0 },
				inventory_component = {
					items = {
						health_potion = 3
					}
				},
				level_component =
				{
					type = "player-core",
					properties = {
						level = 0
					}
				}
			}
		},
		items = {
			attack_gem = {
				components = {
					sprite_component = {
						name = "attackgem",
						spritesheet_name = "game-sprites",
						sprite_id = 2
					}
				},
				attack_bonus_component = {
					type = "powerup",
					properties = {
						action = function(group, entity_name, component_name, component_value_name, existing_component_value)
							local attack_value = 1
							-- TODO: why are we trying to be generic then hardcoding the player here? LOL!
							add_action_log("player", "attack", "+", tostring(attack_value .. " attack"), Game.player_pos.x, Game.player_pos.y)
							set_component_value(group, entity_name, component_name, component_value_name, existing_component_value + attack_value)
						end
					}
				}
			},
			treasure_chest = {
				components = {
					sprite_component = {
						name = "treasure_chest",
						spritesheet_name = "game-sprites",
						sprite_id = 13
					},
					value_component = {
						value = 50
					},
					enemy_bonus_component = {
						type = "bonus",
						properties = {}
					}
				}
			},
			coin = {
				components = {
					sprite_component = {
						name = "coin",
						spritesheet_name = "game-sprites",
						sprite_id = 14
					},
					value_component = {
						value = 25
					}
				},
				total = 100
			},
			healthgem =  {
				components = {
					sprite_component = {
						name = "healthgem",
						spritesheet_name = "game-sprites",
						sprite_id = 1
					},
					health_restoration_component = {
						type = "powerup",
						properties = {
							action = function(group, entity_name, component_name, component_value_name, existing_component_value)
								local health_value = 25
								-- TODO: why are we trying to be generic then hardcoding the player here? LOL!
								add_action_log("player", "health", "+", tostring(health_value .. " health"), Game.player_pos.x, Game.player_pos.y)
								set_component_value(group, entity_name, component_name, component_value_name, existing_component_value + health_value)
							end
						}
					}
				},
				total = 50
			},
			goldencandle = {
				components = {
					sprite_component = {
						name = "goldencandle",
						spritesheet_name = "game-sprites",
						sprite_id = 6
					},
					value_component = { value = 25000 },
					win_component = {
						type = "flags",
						properties = { win = true }
					}
				},
				total = 1
			}
		},
		enemies = {
			spider = {
				components = {
					sprite_component = {
						name = "spider",
						spritesheet_name = "game-sprites",
						sprite_id = 4
					},
					health_component = { health = 20 },
					stats_component = { attack = 1 },
					value_component = { value = 5 },
					aggro_component = {
						type = "enemy-combat",
						properties = { value = 0 }
					}
				},
				total = 50
			},
			crab = {
				components = {
					sprite_component = {
						name = "crab",
						spritesheet_name = "game-sprites",
						sprite_id = 12
					},
					health_component = { health = 30 },
					stats_component = { attack = 2 },
					value_component = { value = 10 },
					aggro_component = {
						type = "enemy-combat",
						properties = { value = 0 }
					}
				},
				total = 30
			},
			bug = {
				components = {
					sprite_component = {
						name = "bug",
						spritesheet_name = "game-sprites",
						sprite_id = 17
					},
					health_component = { health = 25 },
					stats_component = { attack = 2 },
					value_component = { value = 10 },
					aggro_component = {
						type = "enemy-combat",
						properties = { value = 0 }
					}
				},
				total = 25
			},
			lurcher = {
				components = {
					sprite_component = {
						name = "lurcher",
						spritesheet_name = "game-sprites",
						sprite_id = 5
					},
					health_component = { health = 35 },
					stats_component = { attack = 2 },
					value_component = { value = 15 },
					aggro_component = {
						type = "enemy-combat",
						properties = { value = 0 }
					}
				},
				total = 25
			},
			firewalker = {
				components = {
					sprite_component = {
						name = "firewalker",
						spritesheet_name = "game-sprites",
						sprite_id = 21
					},
					health_component = { health = 75 },
					stats_component = { attack = 4 },
					value_component = { value = 40 },
					aggro_component = {
						type = "enemy-combat",
						properties = { value = 0 }
					}
				},
				total = 25
			},
			mantis = {
				components = {
					sprite_component = {
						name = "crimsonshadow",
						spritesheet_name = "game-sprites",
						sprite_id = 54
					},
					health_component = { health = 50 },
					stats_component = { attack = 3 },
					value_component = { value = 30 },
					aggro_component = {
						type = "enemy-combat",
						properties = { value = 0 }
					}
				},
				total = 25
			},
			crimsonshadow = {
				components = {
					sprite_component = {
						name = "crimsonshadow",
						spritesheet_name = "game-sprites",
						sprite_id = 34
					},
					health_component = { health = 85 },
					stats_component = { attack = 5 },
					value_component = { value = 60 },
					aggro_component = {
						type = "enemy-combat",
						properties = { value = 0 }
					}
				},
				total = 20
			},
			purpleblob = {
				components = {
					sprite_component = {
						name = "purpleblob",
						spritesheet_name = "game-sprites",
						sprite_id = 61
					},
					health_component = { health = 95 },
					stats_component = { attack = 6 },
					value_component = { value = 75 },
					aggro_component = {
						type = "enemy-combat",
						properties = { value = 0 }
					}
				},
				total = 15
			},
			orangeblob = {
				components = {
					sprite_component = {
						name = "orangeblob",
						spritesheet_name = "game-sprites",
						sprite_id = 64
					},
					health_component = { health = 100 },
					stats_component = { attack = 7 },
					value_component = { value = 100 },
					aggro_component = {
						type = "enemy-combat",
						properties = { value = 0 }
					}
				},
				total = 10
			}
		}
	}
}

ERROR = false

function started()
	return Game.started == true and Game.won == false and Game.lost == false
end

-- A PID is an X,Y identitier we use as a key into the entities table. This
-- saves us a for loop which is really slow! PID stands for position ID.
-- This function is used to get a PID for a direction the player wants to move.
-- This PID can be used to see if there is anything on the tile we want to move
-- onto.
function get_player_movement_direction_pid(dir)
	local x = 0
	local y = 0

	if(dir == "up") then
		x = Game.player_pos.x
		y = Game.player_pos.y - 1
	elseif(dir == "down") then
		x = Game.player_pos.x
		y = Game.player_pos.y + 1
	elseif(dir == "left") then
		x = Game.player_pos.x - 1
		y = Game.player_pos.y
	elseif(dir == "right") then
		x = Game.player_pos.x + 1
		y = Game.player_pos.y
	end

	return tostring(x .. "_" .. y)
end

function create_pid_from_x_y(x, y)
	return tostring(x .. "_" .. y)
end

function create_entities(entity_table, group, entity_name, entity_type)
	local entities = {}

	for k,v in pairs(entity_table[group]) do
		if(v.total ~= nil) then
			entities = add_entities(group, entity_type, v.components, v.total)
		end
	end

	return entities
end

function get_enemy_by_id(id)
	for k,v in pairs(Game.enemies) do
		if(v.enemy.id == id) then
			return v
		end
	end
end

function xy_falls_within_viewport(x, y)
	if(x >= get_view_port_x() and
	   x <= get_view_port_x() + get_view_port_width() and
	   y >= get_view_port_y() and
	   y <= get_view_port_y() + get_view_port_height()) then
		return true
	end

	return false
end

function add_action_log(who, type, multiplier, value, x, y)
	local id = generate_uuid()

	Game.action_log[id] = {}
	Game.action_log[id]["show"] = 0
	Game.action_log[id]["transparancy"] = 255
	Game.action_log[id]["offset"] = 1
	Game.action_log[id]["who"] = who
	Game.action_log[id]["type"] = type
	Game.action_log[id]["x"] = x
	Game.action_log[id]["y"] = y
	Game.action_log[id]["attack_type"] = type
	Game.action_log[id]["message"] = tostring(multiplier .. value)
end

function move_enemies()
	-- Only move enemies around 20% of the time
	if (get_random_number(1, 100) >= 80) then
		local enemies_new_positions = {}
		local number_of_enemies = 0

		for k, e in pairs(Game.enemies) do
			local x = e.enemy.point.x
			local y = e.enemy.point.y
			local aggro = e.enemy.components.aggro_component.properties.value

			-- only move enemies that are within our viewport
			if(xy_falls_within_viewport(x, y) and aggro ~= true) then
				local direction = get_random_number(1, 4)
				local can_move = false

				if(direction == 1 and is_tile_walkable(x, y, "up", "enemy", { "common", "enemies" })) then
					y = y - 1
					can_move = true
				elseif (direction == 2 and is_tile_walkable(x, y, "down", "enemy", { "common", "enemies" })) then
					y = y + 1
					can_move = true
				elseif (direction == 3 and is_tile_walkable(x, y, "left", "enemy", { "common", "enemies" })) then
					x = x - 1
					can_move = true
				elseif (direction == 4 and is_tile_walkable(x, y, "right", "enemy", { "common", "enemies" })) then
					x = x + 1
					can_move = true
				end

				if (can_move) then
					number_of_enemies = number_of_enemies + 1

					enemies_new_positions[e.enemy.id] = {}
					enemies_new_positions[e.enemy.id]["x"] = x
					enemies_new_positions[e.enemy.id]["y"] = y
				end
			end
		end

		if (number_of_enemies > 0) then
			update_entities_position("enemies", enemies_new_positions)
		end
	end
end

function move_aggro_enemies()
	local number_of_enemies = 0
	local enemy_changes = {}

	for k, e in pairs(Game.enemies) do
		if(e.enemy.components.aggro_component.properties ~= nil and
		   e.enemy.components.aggro_component.properties.value == 1) then
			local x = e.enemy.point.x
			local y = e.enemy.point.y
			local path = {}

			-- FIXME: When the game was turn based it didn't make sense to
			-- return and entire path. This is because we could only ever
			-- execute one step in the path before the player moved again and
			-- the path would have to be recomputed. Now that enemies can move
			-- on their own without the need for the player to first move we
			-- can excute more steps in the path if the player has not moved.
			--
			--
			-- THIS NEEDS TO BE REFACTORED

			-- if(e.enemy.components.aggro_component.properties.pathinfo ~= nil and
			--    (Game.player_pos.x == e.enemy.components.aggro_component.properties.pathinfo.player_pos.x and
			--     Game.player_pos.y == e.enemy.components.aggro_component.properties.pathinfo.player_pos.y)) then
			-- 	print("using path already stored")
			-- 	path = e.enemy.components.aggro_component.properties.pathinfo.path
			-- end

			-- if(next(path) == nil) then
			-- 	path = find_path(x, y, Game.player_pos.x, Game.player_pos.y, 9)

			-- 	if(path.x ~= nil and path.y ~= nil) then
			-- 		--print("finding new path")

			-- 		if(is_xy_blocked(path.x, path.y) ~= true) then
			-- 			number_of_enemies = number_of_enemies + 1
			-- 			enemy_changes[e.enemy.id] = {
			-- 				component_name = "aggro_component",
			-- 				component_key = "pathinfo",
			-- 				component_value = {
			-- 					player_pos = {
			-- 						x = Game.player_pos.x,
			-- 						y = Game.player_pos.y
			-- 					},
			-- 					path = path
			-- 				},
			-- 				position = {
			-- 					x = path.x,
			-- 					y = path.y
			-- 				}
			-- 			}
			-- 		end
			-- 	end
			-- end
		end
	end

	-- if (number_of_enemies > 0) then
	-- 	update_entities("enemies", enemy_changes)
	-- end
end

function initiate_enemy_attack(pid)
	local player_health = Game.player.components.health_component.health
	local enemy_attack = Game.enemies[pid].enemy.components.stats_component.attack
	local enemy_crit_chance =  get_random_number(1, 100) <= 20

	-- enemy strikes next
	if (enemy_crit_chance) then
		-- do crit attack
		local damage = enemy_attack + get_random_number(1, 5) * 2

		add_action_log("enemy", "critical", "-", damage, Game.player_pos.x, Game.player_pos.y)
		set_component_value("common", "player", "health_component", "health", math.floor(player_health - damage))
	else
		-- do normal attack
		local damage = enemy_attack + get_random_number(1, 5)

		add_action_log("enemy", "normal", "-", damage, Game.player_pos.x, Game.player_pos.y)
		set_component_value("common", "player", "health_component", "health", math.floor(player_health - damage))
	end
end

function initiate_attack_sequence(pid)
	-- generate some random numbers for crit chance (player and enemy)
	local player_crit_chance = get_random_number(1, 100) <= 20
	local player_attack = Game.player.components.stats_component.attack
	local player_health = Game.player.components.health_component.health
	local enemy_health = Game.enemies[pid].enemy.components.health_component.health
	local enemy_id = Game.enemies[pid].enemy.id
	local enemy_x = Game.enemies[pid].enemy.point.x
	local enemy_y = Game.enemies[pid].enemy.point.y

	-- player strikes first
	if (player_crit_chance) then
		-- do crit attack
		local damage = player_attack + get_random_number(1, 5) * 2
		set_component_value("enemies", enemy_id, "health_component", "health", math.floor(enemy_health - damage))

		add_action_log("player", "critical", "+", damage, enemy_x, enemy_y)
	else
		-- do normal attack
		local damage = player_attack + get_random_number(1, 5)
		set_component_value("enemies", enemy_id, "health_component", "health", math.floor(enemy_health - damage))

		add_action_log("player", "normal", "+", damage, enemy_x, enemy_y)
	end

	-- check to see if enemy died
	if(enemy_health <= 0) then
		add_action_log("player", "kill", "+", "1 kill", Game.player_pos.x, Game.player_pos.y - 1)

		local enemy_value = Game.enemies[pid].enemy.components.value_component.value
		Game.enemies[pid]=nil
		remove_entity("enemies", enemy_id)

		Game.total_enemies_killed = Game.total_enemies_killed + 1
		-- The score value here should be put into a component and not hard coded
		set_component_value("common", "player", "score_component", "score", Game.player.components.score_component.score + 25)

		Game.entities.items.treasure_chest.components.enemy_bonus_component.properties.action = function()
			local health_boost = get_random_number(1, 100)

			if (health_boost <= 20) then
				local health_bonus = math.floor(player_health + 15 + (2 * health_boost))
				add_action_log("player", "health", "+", tostring(health_boost .. " health"), Game.player_pos.x, Game.player_pos.y - 1)
				set_component_value("common", "player", "health_component", "health", health_bonus)
			end

			add_action_log("player", "score", "+", tostring(enemy_value .. " score"), Game.player_pos.x, Game.player_pos.y)
			set_component_value("common", "player", "score_component", "score", Game.player.components.score_component.score + enemy_value)
		end

		add_entity("treasure_chests", "item", enemy_x, enemy_y, Game.entities.items.treasure_chest.components)
	else
		-- enemy strikes next
		initiate_enemy_attack(pid)
	end

	-- check to see if player died
	if (player_health <= 0) then
		Game.win_lose_message = "You ded son!"
		Game.lost = true
	end
end

function initiate_enemy_preemptive_attack_sequence()
	for k, e in pairs(Game.enemies) do
		local ex = e.enemy.point.x
		local ey = e.enemy.point.y

		-- only move enemies that are within our viewport
		if(xy_falls_within_viewport(ex, ey)) then
		 	if((ex == Game.player_pos.x and ey + 1 == Game.player_pos.y) or
		 	   (ex == Game.player_pos.x and ey - 1 == Game.player_pos.y) or
		 	   (ex + 1 == Game.player_pos.x and ey == Game.player_pos.y) or
		 	   (ex - 1 == Game.player_pos.x and ey == Game.player_pos.y)) then
		 		initiate_enemy_attack(k)
		 	end
		end
	end
end

function aggro_enemy(distance)
	for k, e in pairs(Game.enemies) do
		if(e.enemy.components.aggro_component.properties.value == 0) then
			local ex = e.enemy.point.x
			local ey = e.enemy.point.y

			for i = 1, distance do
				if(xy_falls_within_viewport(ex, ey) and e.enemy.components.aggro_component.properties.value == 0) then
					if((ex == Game.player_pos.x and ey + i == Game.player_pos.y) or
					   (ex == Game.player_pos.x and ey - i == Game.player_pos.y) or
					   (ex + i == Game.player_pos.x and ey == Game.player_pos.y) or
					   (ex - i == Game.player_pos.x and ey == Game.player_pos.y)) then
						set_component_value("enemies", e.enemy.id, "aggro_component", "value", 1)
					end
				end
			end
		end
	end
end

function calculate_health_bar_width(health, starting_health, health_bar_max_width)
	local hw = health_bar_max_width

	if (health <= starting_health) then
		hw = (((health * (100 / starting_health)) * health_bar_max_width) / 100)
	end

	return math.floor(hw)
end

function render_info_bar()
	local player_max_health = Game.player.components.health_component.max_health
	local player_health = Game.player.components.health_component.health
	local player_score = tostring("Score: " .. Game.player.components.score_component.score)
	local player_kills = tostring("Kills: " .. Game.total_enemies_killed)
	local player_level = tostring("Level: " .. Game.player.components.level_component.properties.level)
	local player_score_text_extents = get_text_extents(player_score, "medium")
	local player_health_text_extents = get_text_extents(tostring(player_health), "medium")
	local player_kills_text_extents = get_text_extents(tostring(player_kills), "medium")
	local player_level_text_extents = get_text_extents(tostring(player_level), "medium")

	local p_hw = calculate_health_bar_width(player_health, player_max_health, 200)

	set_draw_color(33, 33, 33, 255)
	draw_filled_rect(10, 10, 200, math.floor(player_health_text_extents.height / 2) + 5)

	if (player_health <= player_max_health / 3) then
		set_draw_color(255, 0, 0, 255) -- red player's health is in trouble
	else
		set_draw_color(8, 138, 41, 255) -- green for player
	end

	draw_filled_rect(10, 10, math.floor(p_hw), math.floor(player_health_text_extents.height / 2) + 5)

	draw_text(tostring(player_health), "medium", math.floor(110 - player_health_text_extents.width / 2), 2)
	draw_text(player_level, "medium", 330, 2)
	draw_text(player_kills, "medium", 640, 2)
	draw_text(player_score, "medium", 960, 2)

	set_draw_color(0, 0, 0, 255)
end

function render_mini_map()
	local window_offset_x = 140
	local window_offset_y = 140
	local offset_x = Game["window_width"] - window_offset_x
	local offset_y = Game["window_height"] - window_offset_y
	for r = 1, Game["map_height"] do
		for c = 1, Game["map_width"] do
			local dx = (c - 1) + offset_x
			local dy = (r - 1) + offset_y

			if (Game.map[r][c] == 0)then
				set_draw_color(255, 255, 255, 128)
				draw_point(dx, dy)
			elseif (Game.map[r][c] == 9) then
				set_draw_color(0, 0, 0, 128)
				draw_point(dx, dy)
			end

			if (dx == Game.goldencandle.point.x + offset_x and dy == Game.goldencandle.point.y + offset_y) then
			 	set_draw_color(255, 255, 0, 255)
			 	draw_filled_rect(dx - 3, dy - 3, 6, 6)
			end

			if (dx == Game.player_pos["x"] + offset_x and dy == Game.player_pos["y"] + offset_y) then
				set_draw_color(0, 255, 0, 255)
				draw_filled_rect(dx - 3, dy - 3, 6, 6)
			end
		end
	end

	set_draw_color(0,0,0,255)
end

function render_entity(entity_group, entity_type, p_id, dx, dy)
	if(entity_group[p_id]) then
		local sprite_id = entity_group[p_id][entity_type]["components"]["sprite_component"]["sprite_id"]
		local spritesheet_name = entity_group[p_id][entity_type]["components"]["sprite_component"]["spritesheet_name"]

		if(entity_type == "enemy") then
			render_health_bar(entity_group[p_id][entity_type], 255, 0, 0, dx, dy)
		end

		draw_sprite(spritesheet_name, sprite_id, dx, dy)
	end
end

function render_health_bar(entity, r, g, b, dx, dy)
	local hw = calculate_health_bar_width(entity["components"]["health_component"]["health"],
										  entity["components"]["health_component"]["max_health"], Game.sprite_info.width)

	if(hw >= 0) then
		set_draw_color(r, g, b, 255)
		draw_filled_rect(dx, dy - 8, hw, 6)
		set_draw_color(0, 0, 0, 255)
	end
end

function render_title_screen()
	draw_graphic(Game.logo_image_path, Game.window_width, 0, 20, true, true, 2)

	draw_sprite("game-sprites", Game.sprite_info["orangeblob_sprite_id"], math.floor(Game.window_width / 2 - 256), 325)
	draw_sprite("game-sprites", Game.sprite_info["crimsonshadow_sprite_id"], math.floor(Game.window_width / 2 - 192), 325)
	draw_sprite("game-sprites", Game.sprite_info["spider_sprite_id"], math.floor(Game.window_width / 2 - 128), 325)
	draw_sprite("game-sprites", Game.sprite_info["lurcher_sprite_id"], math.floor(Game.window_width / 2 - 64), 325)
	draw_sprite("game-sprites", Game.sprite_info["player_sprite_id"], math.floor(Game.window_width / 2), 325)
	draw_sprite("game-sprites", Game.sprite_info["crab_sprite_id"], math.floor(Game.window_width / 2 + 64), 325)
	draw_sprite("game-sprites", Game.sprite_info["firewalker_sprite_id"], math.floor(Game.window_width / 2 + 128), 325)
	draw_sprite("game-sprites", Game.sprite_info["mantis_sprite_id"], math.floor(Game.window_width / 2 + 192), 325)
	draw_sprite("game-sprites", Game.sprite_info["purpleblob_sprite_id"], math.floor(Game.window_width / 2 + 256), 325)

	draw_graphic(Game.start_game_image_path, Game.window_width, 0, 215, true, true, 2)
	draw_graphic(Game.credit_image_path, Game.window_width, 0, 300, true, true, 2)
end

function render_win_or_death_screen()
	local text_extents = get_text_extents(Game.win_lose_message, "large")
	draw_text(Game.win_lose_message, "large", math.floor(Game.window_width / 2 - text_extents.width / 2),
											  math.floor(Game.window_height / 2 - text_extents.height / 2))
	draw_text(tostring("Final Score: " .. Game.player["components"]["score_component"]["score"]), "large", 20, 20)
	draw_text(tostring("Total Enemies Killed: " .. Game.total_enemies_killed), "large", 20, 70)
	draw_text("Press the space bar to play again...", "medium", math.floor(Game.window_width / 2 - text_extents.width / 2 - 150), Game.window_height - 100)
end

function render_action_log()
	for k, action_log in pairs(Game.action_log) do
		if(action_log ~= nil) then
			local text_extents = get_text_extents(action_log.message, "small")
			local x_offset = 0

			if(text_extents.width > 20) then
				x_offset = math.floor(text_extents.width / 2)
			end

			local dx = (action_log.x * Game.sprite_info.width) - (get_view_port_x() * Game.sprite_info.width) - x_offset
			local dy = (action_log.y * Game.sprite_info.height) - (get_view_port_y() * Game.sprite_info.height)

			if (action_log.who == "player") then
			 	if(action_log.type == "health") then
			 		draw_text_with_color(action_log.message, "small", dx, (dy-action_log.offset), 255, 191, 0, action_log.transparancy)
			 	elseif(action_log.type == "score") then
			 		draw_text_with_color(action_log.message, "small", dx, (dy-action_log.offset), 255, 255, 0, action_log.transparancy)
			 	else
	 		 		draw_text_with_color(action_log.message, "small", dx, (dy-action_log.offset), 0, 255, 0, action_log.transparancy)
			 	end
			 elseif (action_log.who == "enemy") then
			 	draw_text_with_color(action_log.message, "small", dx, (dy-action_log.offset), 255, 0, 0, action_log.transparancy)
			end

			set_draw_color(0, 0, 0, 255)
		end
	end
end

function render_treasure_chests()
	if(Game.treasure_chests ~= nil) then
		for k, treasure_chests in pairs(Game.treasure_chests) do
			local tc_x = treasure_chests.item.point.x
			local tc_y = treasure_chests.item.point.y
			local p_id = tostring((tc_x) .. "_" .. (tc_y))
			local dx = (tc_x * Game.sprite_info.width) - (get_view_port_x() * Game.sprite_info.width)
			local dy = (tc_y * Game.sprite_info.height) - (get_view_port_y() * Game.sprite_info.height)

			render_entity(Game.treasure_chests, "item", p_id, dx, dy)
		end
	end
end

-- Engine functions below

function _init()
	add_sprite_sheet("game-sprites", Game.spritesheet_path, Game.sprite_info.width, Game.sprite_info.height)

	add_entity("common", "player", Game.player_pos.x, Game.player_pos.y, Game.entities.player.components)

	generate_map("main", Game.map_width, Game.map_height)
	switch_map("main")

	local pos = generate_random_point({ "common" })
	update_entity_position("common", "player", pos.x, pos.y)

	Game.items = create_entities(Game.entities, "items", "coin", "item")
	Game.enemies = create_entities(Game.entities, "enemies", "spider", "enemy")

	Game.number_of_enemies = 0
	for _ in pairs(Game.enemies) do Game.number_of_enemies = Game.number_of_enemies + 1 end

	for k,v in pairs(Game.items) do
		for k1,v1 in pairs(v) do
			if(v1.components.sprite_component.name == "goldencandle") then
				Game.goldencandle = {
					point = v1.point,
					p_id = tostring(v1.point.x .. "_" .. v1.point.y)
				}
			end
		end
	end

	Game.map = get_map("main", false)
	fov("main")
end

function _update(event, data)
	if(event == "key_event") then
		if data["key"] == "up" and started() then
			if(is_tile_walkable(Game.player_pos.x, Game.player_pos.y, "up", "player", { "common", "enemies" })) then
				update_entity_position("common", "player", Game.player_pos.x, Game.player_pos.y - 1)
			else
				local pid = get_player_movement_direction_pid("up")
				if(Game.enemies[pid]) then
					play_sound("combat")
					initiate_attack_sequence(pid)
				else
					play_sound("bump")
				end
			end
		elseif data["key"] == "down" and started() then
			if(is_tile_walkable(Game.player_pos.x, Game.player_pos.y, "down", "player", { "common", "enemies" })) then
				update_entity_position("common", "player", Game.player_pos.x, Game.player_pos.y + 1)
			else
				local pid = get_player_movement_direction_pid("down")
				if(Game.enemies[pid]) then
					play_sound("combat")
					initiate_attack_sequence(pid)
				else
					play_sound("bump")
				end
			end
		elseif data["key"] == "left" and started()then
			if(is_tile_walkable(Game.player_pos.x, Game.player_pos.y, "left", "player", { "common", "enemies" })) then
				update_entity_position("common", "player", Game.player_pos.x - 1, Game.player_pos.y)
			else
				local pid = get_player_movement_direction_pid("left")
				if(Game.enemies[pid]) then
					play_sound("combat")
					initiate_attack_sequence(pid)
				else
					play_sound("bump")
				end
			end
		elseif data["key"] == "right" and started() then
			if(is_tile_walkable(Game.player_pos.x, Game.player_pos.y, "right", "player", { "common", "enemies" })) then
				update_entity_position("common", "player", Game.player_pos.x + 1, Game.player_pos.y)
			else
				local pid = get_player_movement_direction_pid("right")
				if(Game.enemies[pid]) then
					play_sound("combat")
					initiate_attack_sequence(pid)
				else
					play_sound("bump")
				end
			end
		elseif data["key"] == "space" then
			if (started()) then
				play_sound("warp")
				local pos = generate_random_point({ "common" })
				update_entity_position("common", "player", pos.x, pos.y)
				update_all_entities("enemies", "aggro_component", "value", 0)
			elseif (Game.won == true or Game.lost == true) then
				Game.lost = false
				Game.won = false
				Game.started = true
				reset()
			else
				Game.started = true
			end
		end

		if(Game.started and Game.items[XY_Id]) then
			if (Game.items[XY_Id].item.components.sprite_component.name == "coin") then
				local coin_value = Game.items[XY_Id].item.components.value_component.value
				add_action_log("player", "score", "+", tostring(coin_value .. " score"), Game.player_pos.x, Game.player_pos.y)
				play_sound("coin")
				set_component_value("common", "player", "score_component", "score", Game.player.components.score_component.score + coin_value)
				remove_entity("items", Game.items[XY_Id].item.id)
			elseif (Game.items[XY_Id].item.components.sprite_component.name == "healthgem") then
				play_sound("pickup")
				local action = Game.items[XY_Id].item.components.health_restoration_component.properties.action
				local player_current_health = Game.player.components.health_component.health
				action("common", "player", "health_component", "health", player_current_health)
				remove_entity("items", Game.items[XY_Id].item.id)
			elseif (Game.items[XY_Id].item.components.sprite_component.name == "goldencandle") then
				-- TODO: Use the Lua Component associated with the goldencandle to determine actions to take
				Game.win_lose_message = "YOU WIN!!!"
				Game.won = true
				Game.started = false
				set_component_value("common", "player", "score_component", "score",
					Game.player.components.score_component.score +
					Game.items[XY_Id].item.components.value_component.value)
				remove_entity("items", Game.items[XY_Id].item.id)
			end
		elseif (Game.started and Game.treasure_chests ~= nil and Game.treasure_chests[XY_Id]) then
			if (Game.treasure_chests[XY_Id].item.components.sprite_component.name == "treasure_chest") then
				play_sound("pickup")
				Game.treasure_chests[XY_Id].item.components.enemy_bonus_component.properties.action()
				remove_entity("treasure_chests", Game.treasure_chests[XY_Id].item.id)
			end
		end
	elseif (event == "entity_event") then
		if (data["player"] ~= nil) then
			Game.player = data.player
			Game.player_id = data.player.id

			if(Game.player_pos.x ~= data.player.point.x or
			   Game.player_pos.y ~= data.player.point.y) then
				Game.player_pos.x = data.player.point.x
				Game.player_pos.y = data.player.point.y
				XY_Id = create_pid_from_x_y(Game.player_pos.x,Game.player_pos.y)
				fov("main")
			end
		elseif (data["enemy"] ~= nil) then
		 	local enemy_pid = create_pid_from_x_y(data.enemy.point.x, data.enemy.point.y)
		 	Game.enemies[enemy_pid].enemy = data["enemy"]
		else
			Game[data["entity_group_name"]] = data["entity_group"]
		end
	elseif (event == "light_map") then
	 	Game.light_map = data
	end
end

function _render(delta_time)
	if (ERROR) then
		return
	end

	if(Game.started == false and Game.won == false and Game.lost == false) then
			render_title_screen()
		else if (Game.started == true and Game.won == false and Game.lost == false) then
			render_treasure_chests()

			for r = 1, get_view_port_height() do
				for c = 1, get_view_port_width() do
					local p_id = tostring((c-1) .. "_" .. (r-1))
					local dx = ((c-1) * Game.sprite_info.width) - (get_view_port_x() * Game.sprite_info.width)
					local dy = ((r-1) * Game.sprite_info.height) - (get_view_port_y() * Game.sprite_info.height)

					if(Game.light_map[r][c] == 2) then
						draw_sprite("game-sprites", Game.map[r][c], dx, dy)

						render_entity(Game.items, "item", p_id, dx, dy)
						render_entity(Game.enemies, "enemy", p_id, dx, dy)

						if(Game.player_pos["x"] == (c-1) and Game.player_pos["y"] == (r-1)) then
							render_health_bar(Game.player, 8, 138, 41, dx, dy)
							draw_sprite("game-sprites", Game.sprite_info["player_sprite_id"], dx, dy)
						end
					else
						draw_sprite("game-sprites", Game.sprite_info["hidden_sprite_id"] , dx, dy)
					end
				end
			end

			render_info_bar()
			render_mini_map()
			render_action_log()
		else
			render_win_or_death_screen()
		end
	end
end

function _tick(delta_time)
	Game.health_recovery_timer = Game.health_recovery_timer + delta_time
	Game.preemptive_enemy_attack_timer = Game.preemptive_enemy_attack_timer + delta_time
	Game.level_check_timer = Game.level_check_timer + delta_time
	Game.logic_timer = Game.logic_timer + delta_time

	if(Game.health_recovery_timer >= 2) then
		Game.health_recovery_timer = 0

		if(Game.debug) then
			set_component_value("common", "player", "health_component", "health", 10000)
		else
			if(Game.player.components.health_component.health < Game.player.components.health_component.max_health) then
				set_component_value("common", "player", "health_component", "health", math.floor(Game.player.components.health_component.health + Game.health_recovery))
			end
		end
	end

	for k, action_log in pairs(Game.action_log) do
		action_log.show = action_log.show + delta_time
		action_log.offset = action_log.offset + 10
		action_log.transparancy = action_log.transparancy - 10

		if(action_log.transparancy <= 10) then
			action_log.transparancy = 10
		end

		if(action_log.show >= 2) then
			Game.action_log[k] = nil
		end
	end

	if(Game.preemptive_enemy_attack_timer >= .5) then
		Game.preemptive_enemy_attack_timer = 0
		aggro_enemy(5)
		initiate_enemy_preemptive_attack_sequence()
	end

	if(Game.logic_timer >= .5) then
		Game.logic_timer = 0
		move_enemies()
		move_aggro_enemies()
	end

	if(Game.level_check_timer >= 5 and Game.total_enemies_killed >= 10) then
		Game.level_check_timer = 0
		-- This is a place holder just to figure out how I could work in logic
		-- to make leveling work on a number of enemies killed basis. This will
		-- morph into an experience based leveling system with hard to kill
		-- monsters giving more XP and thereby allowing faster leveling.
		local level = math.floor(Game.total_enemies_killed / 10)

		if(level >= 1 and Game.player.components.level_component.properties.level < level) then
			set_component_value("common", "player", "level_component", "level", Game.player.components.level_component.properties.level + 1)
			set_component_value("common", "player", "stats_component", "attack", Game.player.components.stats_component.attack + 1)
		end
	end
end

function _error(err)
	ERROR = true
	print("An error occurred: " .. err)
end
