-- roguely.lua (20 June 2023)
--
-- MIT License
--
-- Copyright (c) 2023 Frank Hale <frankhaledevelops@gmail.com>
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
    window_title = "Roguely (2023 Edition) - A simple Roguelike in C++/Lua/SDL2",
    window_icon_path = "assets/icon.png",
    window_width = 1280,
    window_height = 640,
    map_width = 100,
    map_height = 100,
    spritesheet_name = "roguely-x",
    spritesheet_path = "assets/roguely-x.png",
    spritesheet_sprite_width = 8,
    spritesheet_sprite_height = 8,
    spritesheet_sprite_scale_factor = 4,
    font_path = "assets/NESCyrillic.ttf",
    soundtrack_path = "assets/ExitExitProper.mp3",
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
        walk = "assets/sounds/walk.wav"
    },
    debug = false,
    -- These are used for map rendering. Maps are simple and just a wall or a
    -- floor tile. This is here so that we aren't hard coding sprite ids in the
    -- render function.
    sprite_ids = {
        wall = 67,
        floor = 74
    },
    action_log = {},
    health_recovery_timer = 0,
    level_growth_rate = 1.2,
    level_xp_start = 25,
    level_cap = 30,
    level_data = {},
    entities = {
        player = {
            components = {
                current_scene_component = {
                    name = "title_scene"
                },
                sprite_component = {
                    name = "player",
                    spritesheet_name = "roguely-x",
                    sprite_id = 15,
                    blink = false,
                    render = function(self, game, player, dx, dy, scale_factor)
                        if (self.blink) then
                            set_highlight_color(self.spritesheet_name, 255, 0, 0)
                        end

                        draw_sprite_scaled(self.spritesheet_name, self.sprite_id, dx, dy, scale_factor)

                        if (self.blink) then
                            self.blink = false
                            reset_highlight_color(self.spritesheet_name)
                        end

                        player.components.healthbar_component:render(game, player, dx-2, dy, 8, 138, 41)

                        if(game.debug) then
                            display_position(player, dx, dy)
                        end
                    end
                },
                position_component = { x = 0, y = 0 },
                stats_component = {
                    max_health = 50,
                    health = 50,
                    health_recovery = 10,
                    attack = 5,
                    crit_chance = 2,
                    crit_multiplier = 2,
                    score = 0,
                    kills = 0,
                    level = 0,
                    experience = 0,
                    level_up = function(self, game, player)
                        self.level = self.level + 1
                        self.experience = 0
                        self.max_health = math.floor(self.max_health * game.level_growth_rate + self.level)
                        self.health = self.max_health
                        self.attack = math.floor(self.attack * game.level_growth_rate)
                        self.crit_chance = math.floor(self.crit_chance * game.level_growth_rate)
                        self.crit_multiplier = math.floor(self.crit_multiplier * game.level_growth_rate)
                    end,
                    add_kill = function(self, player, mob)
                        self.kills = self.kills + 1
                        self.experience = self.experience + math.floor(mob.components.stats_component.max_health / 10)
                        -- print(string.format("XP: %d", self.experience))
                    end,
                    add_score = function(self, player, score)
                        self.score = self.score + score

                        add_action_log("player", "score", "+",
                            score,
                            player.components.position_component.x,
                            player.components.position_component.y)
                    end,
                    add_health = function(self, player, health)
                        self.health = math.min(self.health + health, self.max_health)

                        player.components.sprite_component.blink = false

                        add_action_log("player", "health", "+",
                            health,
                            player.components.position_component.x,
                            player.components.position_component.y)
                    end,
                    add_xp = function(self, player, xp)
                        self.experience = self.experience + xp
                    end,
                    take_damage = function(self, player, damage)
                        --print(string.format("PLAYER TAKE DAMAGE: %d", damage))
                        player.components.sprite_component.blink = true

                        player.components.stats_component.health = player.components.stats_component.health - damage
                        if(player.components.stats_component.health <= 0) then
                            player.components.stats_component.health = 0

                            player.components.current_scene_component.name = "end_scene"
                        end
                    end,
                    inflict_damage = function(self, player, entities)
                        local entity_attack = get_random_number(1, player.components.stats_component.attack)
                        if(is_critical_attack) then
                            damage = entity_attack * player.components.stats_component.crit_multiplier
                        else
                            damage = entity_attack
                        end

                        entities.mobs[player.components.combat_component.mob].components.stats_component:take_damage(player, entities.mobs[player.components.combat_component.mob], damage)

                        add_action_log("player", "attack", "+",
                             damage,
                             entities.mobs[player.components.combat_component.mob].components.position_component.x,
                             entities.mobs[player.components.combat_component.mob].components.position_component.y)
                    end
                },
                healthbar_component = {
                    render = function(self, game, entity, dx, dy, r, g, b)
                        local hw = calculate_health_bar_width(
                                        entity.components.stats_component.health,
                                        entity.components.stats_component.max_health, 32)

                        if (hw >= 0) then
                            set_draw_color(r, g, b, 255)
                            draw_filled_rect(dx, dy - 8, hw, 6)
                            set_draw_color(0, 0, 0, 255)
                        end
                    end
                },
                tick_component = {
                    tick = function(self, game, player)
                        if (player.components.stats_component.health < player.components.stats_component.max_health) then
                            player.components.stats_component.health = math.min(player.components.stats_component.health + player.components.stats_component.health_recovery, player.components.stats_component.max_health)
                        end
                    end
                }
            }
        },
        -- We don't need a separate full copy of the entity definition for each
        -- mob because all that changes are some stats, sprite and name. When
        -- the mobs are spawned the components will be built on the fly.
        enemies = {
            orangeblob = {
                sprite_id = 1,
                components = {
                    -- Additional components will be generated at spawn time
                    -- since there isn't much that varies between enemies
                    stats_component = {
                        max_health = 45,
                        health = 45,
                        attack = 2,
                        crit_chance = 3,
                        crit_multiplier = 3
                    }
                }
            },
            greenblob = {
                sprite_id = 2,
                components = {
                    stats_component = {
                        max_health = 30,
                        health = 30,
                        attack = 3,
                        crit_chance = 3,
                        crit_multiplier = 2
                    }
                }
            },
            ant = {
                sprite_id = 3,
                components = {
                    stats_component = {
                        max_health = 20,
                        health = 20,
                        attack = 2,
                        crit_chance = 4,
                        crit_multiplier = 3
                    }
                }
            },
            crimsonshadow = {
                sprite_id = 4,
                components = {
                    stats_component = {
                        max_health = 40,
                        health = 40,
                        attack = 4,
                        crit_chance = 3,
                        crit_multiplier = 2
                    }
                }
            },
            spider = {
                sprite_id = 5,
                components = {
                    stats_component = {
                        max_health = 20,
                        health = 20,
                        attack = 2,
                        crit_chance = 2,
                        crit_multiplier = 2
                    }
                }
            },
            lurcher = {
                sprite_id = 6,
                components = {
                    stats_component = {
                        max_health = 35,
                        health = 35,
                        attack = 3,
                        crit_chance = 2,
                        crit_multiplier = 2
                    }
                }
            },
            firewalker = {
                sprite_id = 7,
                components = {
                    stats_component = {
                        max_health = 45,
                        health = 45,
                        attack = 5,
                        crit_chance = 3,
                        crit_multiplier = 3
                    }
                }
            },
            mantis = {
                sprite_id = 8,
                components = {
                    stats_component = {
                        max_health = 20,
                        health = 20,
                        attack = 2,
                        crit_chance = 5,
                        crit_multiplier = 2
                    }
                }
            },
            crab = {
                sprite_id = 9,
                components = {
                    stats_component = {
                        max_health = 30,
                        health = 30,
                        attack = 3,
                        crit_chance = 3,
                        crit_multiplier = 3
                    }
                }
            },
            purpleblob = {
                sprite_id = 10,
                components = {
                    stats_component = {
                        max_health = 55,
                        health = 55,
                        attack = 5,
                        crit_chance = 3,
                        crit_multiplier = 4
                    }
                }
            },
            bug = {
                sprite_id = 11,
                components = {
                    stats_component = {
                        max_health = 25,
                        health = 25,
                        attack = 4,
                        crit_chance = 2,
                        crit_multiplier = 2
                    }
                }
            },
            beetle = {
                sprite_id = 12,
                components = {
                    stats_component = {
                        max_health = 75,
                        health = 75,
                        attack = 3,
                        crit_chance = 3,
                        crit_multiplier = 6
                    }
                }
            },
            roach = {
                sprite_id = 13,
                components = {
                    stats_component = {
                        max_health = 100,
                        health = 100,
                        attack = 2,
                        crit_chance = 2,
                        crit_multiplier = 2
                    }
                }
            },
            scorpion = {
                sprite_id = 14,
                components = {
                    stats_component = {
                        max_health = 120,
                        health = 120,
                        attack = 5,
                        crit_chance = 6,
                        crit_multiplier = 5
                    }
                }
            }
        },
        ui = {
            title_scene = {
                components = {
                    render_component = {
                        render = function(self, game, player, entities, dx, dy)
                            draw_graphic(game.logo_image_path, game.window_width, 0, 20, true, 2)

                            local counter = 0
                            for key, value in pairs(game.entities.enemies) do
                                counter = counter + 64
                                draw_sprite_scaled(game.spritesheet_name, value.components.sprite_component.sprite_id,
                                                    math.floor(game.window_width - 200 - counter), 300, game.spritesheet_sprite_scale_factor)
                            end

                            draw_graphic(game.start_game_image_path, game.window_width, 0, 180, true, 2)
                            draw_graphic(game.credit_image_path, game.window_width, 0, 230, true, 2)
                        end
                    }
                }
            },
            end_scene = {
                components = {
                    render_component = {
                        render = function(self, game, player, entities, dx, dy)
                            local text_extents = get_text_extents("The End")
                            draw_text("The End",
                                math.floor(game.window_width / 2 - text_extents.width / 2),
                                math.floor(game.window_height / 2 - text_extents.height / 2))
                            draw_text(string.format("Final Score: %d", player.components.stats_component.score), 20, 20)
                            draw_text(string.format("Total Enemies Killed: %d", player.components.stats_component.kills), 20, 70)
                            --draw_text("Press the space bar to play again...",
                            --math.floor(game.window_width / 2 - text_extents.width / 2 - 190),
                            --game.window_height - 150)
                        end
                    }
                }
            },
            hud = {
                components = {
                    render_component = {
                        render = function(self, game, player, entities, dx, dy)
                            draw_filled_rect_with_color(0, 0, game.window_width, 50, 25, 25, 25, 150)
                            draw_text("Roguely!", 10, 10)
                            local score_str = string.format("Score: %d", player.components.stats_component.score)
                            local score_text_extents = get_text_extents(score_str)
                            draw_text(score_str, math.floor(game.window_width / 2 - score_text_extents.width / 2), 10)
                            local kills_str = string.format("Kills: %d", player.components.stats_component.kills)
                            local kills_text_extents = get_text_extents(kills_str)
                            draw_text(kills_str, game.window_width - kills_text_extents.width - 175, 10)
                            local level_str = string.format("Level: %d", player.components.stats_component.level)
                            local level_text_extents = get_text_extents(level_str)
                            draw_text(level_str, game.window_width - level_text_extents.width - 30, 10)
                        end
                    }
                }
            },
            minimap = {
                components = {
                    render_component = {
                        render = function(self, game, player, entities, dx, dy)
                            draw_full_map("level1",
                                Game.window_width - 135,
                                Game.window_height - 135,
                                225,
                                function(dy, dx, cell_id)
                                    if (cell_id == 0) then
                                        set_draw_color(128, 128, 128, 255)
                                        draw_point(dx, dy)
                                    elseif (cell_id == 1) then
                                        set_draw_color(0, 0, 0, 255)
                                        draw_point(dx, dy)
                                    end

                                    if (dx == player.components.position_component.x and dy == player.components.position_component.y) then
                                        set_draw_color(0, 255, 0, 255)
                                        draw_filled_rect(dx - 2, dy - 2, 3, 3)
                                    end
                                end)
                        end
                    }
                }
            }
        },
        items = {
            coin = {
                components = {
                    position_component = { x = 0, y = 0 },
                    sprite_component = {
                        name = "coin",
                        spritesheet_name = "roguely-x",
                        sprite_id = 44,
                        render = function(self, game, entity, dx, dy, scale_factor)
                            draw_sprite_scaled(self.spritesheet_name, self.sprite_id, dx, dy, scale_factor)

                            if(game.debug) then
                                display_position(entity, dx, dy)
                            end
                        end
                    },
                    value_component = {value = 25}
                }
            },
            health_gem = {
                components = {
                    position_component = { x = 0, y = 0 },
                    sprite_component = {
                        name = "health_gem",
                        spritesheet_name = "roguely-x",
                        sprite_id = 32,
                        render = function(self, game, entity, dx, dy, scale_factor)
                            draw_sprite_scaled(self.spritesheet_name, self.sprite_id, dx, dy, scale_factor)

                            if(game.debug) then
                                display_position(entity, dx, dy)
                            end
                        end
                    },
                    value_component = {value = 25}
                }
            },
            treasure_chests = {
                ordinary = {
                    components = {
                        position_component = { x = 0, y = 0 },
                        sprite_component = {
                            name = "ordinary treasure chest",
                            spritesheet_name = "roguely-x",
                            sprite_id = 46,
                            render = function(self, game, entity, dx, dy, scale_factor)
                                draw_sprite_scaled(self.spritesheet_name, self.sprite_id, dx, dy, scale_factor)

                                if(game.debug) then
                                    display_position(entity, dx, dy)
                                end
                            end
                        },
                        value_component = {value = 50}
                    }
                },
                common = {
                    components = {
                        position_component = { x = 0, y = 0 },
                        sprite_component = {
                            name = "common treasure chest",
                            spritesheet_name = "roguely-x",
                            sprite_id = 47,
                            render = function(self, game, entity, dx, dy, scale_factor)
                                draw_sprite_scaled(self.spritesheet_name, self.sprite_id, dx, dy, scale_factor)

                                if(game.debug) then
                                    display_position(entity, dx, dy)
                                end
                            end
                        },
                        value_component = {value = 70}
                    }
                },
                ornate = {
                    components = {
                        position_component = { x = 0, y = 0 },
                        sprite_component = {
                            name = "ornate treasure chest",
                            spritesheet_name = "roguely-x",
                            sprite_id = 62,
                            render = function(self, game, entity, dx, dy, scale_factor)
                                draw_sprite_scaled(self.spritesheet_name, self.sprite_id, dx, dy, scale_factor)

                                if(game.debug) then
                                    display_position(entity, dx, dy)
                                end
                            end
                        },
                        value_component = {value = 90}
                    }
                },
                exquisite = {
                    components = {
                        position_component = { x = 0, y = 0 },
                        sprite_component = {
                            name = "exquisite treasure chest",
                            spritesheet_name = "roguely-x",
                            sprite_id = 63,
                            render = function(self, game, entity, dx, dy, scale_factor)
                                draw_sprite_scaled(self.spritesheet_name, self.sprite_id, dx, dy, scale_factor)

                                if(game.debug) then
                                    display_position(entity, dx, dy)
                                end
                            end
                        },
                        value_component = {value = 150}
                    }
                }
            },
            goldencandle = {
                components = {
                    sprite_component = {
                        name = "goldencandle",
                        spritesheet_name = "roguely-x",
                        sprite_id = 77,
                        render = function(self, game, entity, dx, dy, scale_factor)
                            draw_sprite_scaled(self.spritesheet_name, self.sprite_id, dx, dy, scale_factor)

                            if(game.debug) then
                                display_position(entity, dx, dy)
                            end
                        end
                    },
                    value_component = {value = 25000}
                },
                total = 1
            }
        }
    }
}

function calculate_health_bar_width(health, max_health,
                                    health_bar_max_width)
    local hw = health_bar_max_width

    if (health <= max_health) then
        hw = (((health * (100 / max_health)) * health_bar_max_width) / 100)
    end

    return math.floor(hw)
end

function spawn_player()
    local player_spawn_point = get_random_point_on_map()
    Game.entities.player.components.position_component = { x = player_spawn_point.x, y = player_spawn_point.y }
    add_entity("common", "player", Game.entities.player.components)
    update_player_viewport(Game.entities.player.components.position_component.x,
            Game.entities.player.components.position_component.y,
            Game.viewport_width,
            Game.viewport_height)
end

function spawn_coins()
    for i = 1, 50 do
        local spawn_point = get_random_point_on_map()
        Game.entities.items.coin.components.position_component = { x = spawn_point.x, y = spawn_point.y }
        add_entity("items", "coin", Game.entities.items.coin.components)
    end
end

function spawn_health_gems()
    for i = 1, 25 do
        local spawn_point = get_random_point_on_map()
        Game.entities.items.health_gem.components.position_component = { x = spawn_point.x, y = spawn_point.y }
        add_entity("items", "health_gem", Game.entities.items.health_gem.components)
    end
end

function spawn_mobs()
    local add_mob_components = function(mob, mob_spawn_point)
        Game.entities.enemies[mob].components.stats_component["take_damage"] = function(self, player, entity, damage)
            --print(string.format("MOB TAKE DAMAGE: %d", damage))

            entity.components.sprite_component.blink = true

            entity.components.stats_component.health = entity.components.stats_component.health - damage
            if(entity.components.stats_component.health <= 0) then
                entity.components.stats_component.health = 0
            end
        end
        Game.entities.enemies[mob].components.stats_component["inflict_damage"] = function(self, player, entities)
            local mob = entities.mobs[player.components.combat_component.mob]
            local entity_attack = get_random_number(1, mob.components.stats_component.attack)
            if(is_critical_attack) then
                damage = entity_attack * mob.components.stats_component.crit_multiplier
            else
                damage = entity_attack
            end

            add_action_log("mob", "attack", "-",
                damage,
                player.components.position_component.x,
                player.components.position_component.y)

            player.components.stats_component:take_damage(player, damage)
        end
        Game.entities.enemies[mob].components.sprite_component = {
            name = mob,
            spritesheet_name = Game.spritesheet_name,
	        sprite_id = Game.entities.enemies[mob].sprite_id,
            blink = false,
            render = function(self, game, entity, dx, dy, scale_factor)
                --draw_sprite_scaled(self.spritesheet_name, self.sprite_id, dx, dy, scale_factor)
                if (self.blink) then
                    set_highlight_color(self.spritesheet_name, 128, 128, 128)
                end

                draw_sprite_scaled(self.spritesheet_name, self.sprite_id, dx, dy, scale_factor)

                if (self.blink) then
                    self.blink = false
                    reset_highlight_color(self.spritesheet_name)
                end

                entity.components.healthbar_component:render(game, entity, dx, dy, 255, 0, 0)

                if(game.debug) then
                    display_position(entity, dx, dy)
                end
            end
        }
        Game.entities.enemies[mob].components.healthbar_component = {
            render = function(self, game, entity, dx, dy, r, g, b)
                local hw = calculate_health_bar_width(
                                entity.components.stats_component.health,
                                entity.components.stats_component.max_health, 32)
                if (hw >= 0) then
                    set_draw_color(r, g, b, 255)
                    draw_filled_rect(dx, dy - 8, hw, 6)
                    set_draw_color(0, 0, 0, 255)
                end
            end
        }
        Game.entities.enemies[mob].components.position_component = { x = mob_spawn_point.x, y = mob_spawn_point.y }
    end

    for key, value in pairs(Game.entities.enemies) do
        add_mob_components(key, {x = 0, y = 0})
    end

    for i = 1, 50 do
        local mob_spawn_point = get_random_point_on_map()
        local mob = get_random_key_from_table(Game.entities.enemies)
        add_mob_components(mob, mob_spawn_point)
        add_entity("mobs", mob, Game.entities.enemies[mob].components)
    end
end

function spawn_treasure_chest(name, spawn_point)
    Game.entities.items.treasure_chests[name].components.position_component = { x = spawn_point.x, y = spawn_point.y }
    add_entity("items", name, Game.entities.items.treasure_chests[name].components)
end

function spawn_golden_candle()
    local spawn_point = get_random_point_on_map()
    Game.entities.items.goldencandle.components.position_component = { x = spawn_point.x, y = spawn_point.y }
    add_entity("items", "goldencandle", Game.entities.items.goldencandle.components)
end

function display_position(entity, dx, dy)
    local position_display = string.format("(%d,%d)", entity.components.position_component.x, entity.components.position_component.y)
    local text_extents = get_text_extents(position_display)
    draw_text(position_display,
        math.floor(dx - text_extents.width),
        math.floor(dy - text_extents.height) + 22)
end

function add_action_log(who, type, multiplier, value, x, y)
    local coords = map_to_world(x, y, Game.spritesheet_name)
    local r = 0
    local g = 0
    local b = 0

    if(type == "score") then
        r = 255
        g = 255
        b = 0
    elseif(multiplier == "+") then
        r = 0
        g = 255
        b = 0
    elseif (multiplier == "-") then
        r = 255
        g = 0
        b = 0
    end

    local text_extents = get_text_extents(string.format("%s%d", multiplier, value))

    -- print(string.format("coords.x: %d", coords.x))
    -- print(string.format("coords.y: %d", coords.y))
    -- print(string.format("text_extents.width: %d", text_extents.width / 2))
    -- print(string.format("text_extents.height: %d", text_extents.height / 2))

    Game.action_log[generate_uuid()] = {
        transparancy = 255,
        who = who,
        type = type,
        multiplier = multiplier,
        x = math.floor(coords.x - (text_extents.width / 4)),
        y = coords.y - math.floor(text_extents.height*1.5),
        r = r,
        g = g,
        b = b,
        message = string.format("%s%d", multiplier, value)
    }
end

function render_action_log()
    for action_log_key, action_log_value in pairs(Game.action_log) do
        draw_text_with_color(action_log_value.message,
            action_log_value.x,
            action_log_value.y,
            action_log_value.r, action_log_value.g, action_log_value.b, action_log_value.transparancy)

        action_log_value.y = action_log_value.y - 10
        action_log_value.transparancy = math.max(action_log_value.transparancy - 50, 1)

        if(action_log_value.transparancy == 1) then
            Game.action_log[action_log_key] = nil
        end
    end
end

function _init()
    for level = 1, Game.level_cap do
        local experience = math.floor(Game.level_xp_start * (Game.level_growth_rate ^ (level - 1)))
        Game.level_data[level] = { level = level, experience = experience }

        -- print("level: " .. level .. " experience: " .. experience)
    end

    add_font("large", Game.font_path, 32)
    add_font("medium", Game.font_path, 24)
    add_font("small", Game.font_path, 16)

    set_font("large")

    generate_map("level1", Game.map_width, Game.map_height)

    add_entity("ui", "title_scene", Game.entities.ui.title_scene.components)
    add_entity("ui", "end_scene", Game.entities.ui.end_scene.components)
    add_entity("ui", "hud", Game.entities.ui.hud.components)
    add_entity("ui", "minimap", Game.entities.ui.minimap.components)

    spawn_player()
    spawn_mobs()
    spawn_coins()
    spawn_health_gems()
    spawn_golden_candle()

    add_system("render_system", render_system)
    add_system("keyboard_input_system", keyboard_input_system)
    add_system("combat_system", combat_system)
    add_system("leveling_system", leveling_system)
    add_system("loot system", loot_system)
    add_system("tick_system", tick_system)
    add_system("mob_movement_system", mob_movement_system)
end

function render_system(delta_time, player, entities, entities_in_viewport)
    if player.components.current_scene_component.name == "game" then
        draw_visible_map("level1", Game.spritesheet_name,
            function(rows, cols, dx, dy, cell_id, light_cell, scale_factor)
                local sprite_id = 0
                if cell_id == 0 then
                    sprite_id = Game.sprite_ids.wall;
                elseif cell_id == 1 then
                    sprite_id = Game.sprite_ids.floor;
                end

                if(player.components.position_component.x == cols and player.components.position_component.y == rows) then
                    player.components.sprite_component:render(Game, player, dx, dy, scale_factor)
                else
                    if(light_cell == 1) then
                        -- wall or floor, these are not entities
                        draw_sprite_scaled(Game.spritesheet_name, sprite_id, dx, dy, scale_factor)

                        for key, value in pairs(entities.items) do
                            if value.components.position_component.x == cols and value.components.position_component.y == rows then
                                value.components.sprite_component:render(Game, value, dx, dy, scale_factor)
                            end
                        end

                        for key, value in pairs(entities.mobs) do
                            if value.components.position_component.x == cols and value.components.position_component.y == rows then
                                value.components.sprite_component:render(Game, value, dx, dy, scale_factor)
                            end
                        end
                    end
                end
            end)

        render_action_log()

        local minimap = find_entity_with_name("ui", "minimap")
        minimap.components.render_component:render(Game, player, entities, dx, dy)
        local hud = find_entity_with_name("ui", "hud")
        hud.components.render_component:render(Game, player, entities, dx, dy)
    elseif player.components.current_scene_component.name == "title_scene" then
        local title_scene = find_entity_with_name("ui", "title_scene")
        title_scene.components.render_component:render(Game, player, entities, dx, dy)
    elseif player.components.current_scene_component.name == "end_scene" then
        local end_scene = find_entity_with_name("ui", "end_scene")
        end_scene.components.render_component:render(Game, player, entities, dx, dy)
    end
end

function keyboard_input_system(key, player, entities, entities_in_viewport)
    if player.components.current_scene_component.name == "game" then
        local walk = false
        local new_position = { x = player.components.position_component.x, y = player.components.position_component.y}
        local blocked_mob = {}

        if Game.keycodes[key] == "up" or Game.keycodes[key] == "w" then
            new_position.y = player.components.position_component.y - 1
            blocked_mob = get_blocked_points("mobs", player.components.position_component.x,
                    player.components.position_component.y, "up")
            walk = true
        elseif Game.keycodes[key] == "down" or Game.keycodes[key] == "s" then
            new_position.y = player.components.position_component.y + 1
            blocked_mob = get_blocked_points("mobs", player.components.position_component.x,
                    player.components.position_component.y, "down")
            walk = true
        elseif Game.keycodes[key] == "left" or Game.keycodes[key] == "a" then
            new_position.x = player.components.position_component.x - 1
            blocked_mob = get_blocked_points("mobs", player.components.position_component.x,
                    player.components.position_component.y, "left")
            walk = true
        elseif Game.keycodes[key] == "right" or Game.keycodes[key] == "d" then
            new_position.x = player.components.position_component.x + 1
            blocked_mob = get_blocked_points("mobs", player.components.position_component.x,
                    player.components.position_component.y, "right")
            walk = true
        elseif Game.keycodes[key] == "space" then
            play_sound("warp")
            local pos = get_random_point_on_map()
            player.components.position_component = { x = pos.x, y = pos.y }

            update_player_viewport(
                player.components.position_component.x,
                player.components.position_component.y,
                Game.viewport_width, Game.viewport_height)
        end

        local adjacent_points = get_adjacent_points(player.components.position_component.x, player.components.position_component.y)
        for key, value in pairs(adjacent_points) do
            if(value.x == new_position.x and value.y == new_position.y and value.blocked) then
                play_sound("bump")
                walk = false
            end
        end

        if blocked_mob.entity_name ~= nil then
            play_sound("combat")
            player.components.combat_component = {
                mob = blocked_mob.entity_full_name
            }
        elseif walk then
            player.components.position_component = { x = math.max(0, math.min(new_position.x, Game.map_width - 1)),
                y = math.max(0, math.min(new_position.y, Game.map_height - 1)) }

            update_player_viewport(
                player.components.position_component.x,
                player.components.position_component.y,
                Game.viewport_width, Game.viewport_height)
            play_sound("walk")

            get_overlapping_points("player", player.components.position_component.x,
                player.components.position_component.y,
                function(entity_full_name, entity_name, components)
                    -- FIXME: This logic should be in the loot system

                    if (entity_name == "health_gem") then
                        play_sound("pickup")
                        player.components.health_update_component = {
                            entity_group = "items",
                            entity_id = entities.items[entity_full_name].id,
                            entity_full_name = entity_full_name,
                            value = entities.items[entity_full_name].components.value_component.value
                        }
                    elseif (entity_name == "ordinary" or
                            entity_name == "common" or
                            entity_name == "ornate" or
                            entity_name == "exquisite" or
                            entity_name == "coin" or
                            entity_name == "goldencandle") then
                        play_sound("pickup")
                        player.components.score_update_component = {
                            entity_group = "items",
                            entity_id = entities.items[entity_full_name].id,
                            entity_full_name = entity_full_name,
                            value = entities.items[entity_full_name].components.value_component.value
                        }

                        if(entity_name == "goldencandle") then
                            player.components.current_scene_component.name = "end_scene"
                        end
                    end
                end)
        end
    elseif player.components.current_scene_component.name == "title_scene" then
        if Game.keycodes[key] == "space" then
            player.components.current_scene_component.name = "game"
        end
    end
end

function combat_system(player, entities, entities_in_viewport)
    if(player.components.combat_component) then
        local is_critical_attack = get_random_number(1, 100) <= player.components.stats_component.crit_chance
        local player_attack = get_random_number(1, player.components.stats_component.attack)
        local damage = 0

        player.components.stats_component:inflict_damage(player, entities)
        entities.mobs[player.components.combat_component.mob].components.stats_component:inflict_damage(player, entities)

        if(entities.mobs[player.components.combat_component.mob].components.stats_component.health <= 0) then
            player.components.stats_component:add_kill(player, entities.mobs[player.components.combat_component.mob])

            player.components.score_update_component = {
                entity_group = "mobs",
                entity_id = entities.mobs[player.components.combat_component.mob].id,
                entity_full_name = player.components.combat_component.mob,
                value = entities.mobs[player.components.combat_component.mob].components.stats_component.max_health
            }
        end

        remove_component("common", "player", "combat_component")
    end
end

function leveling_system(player, entities, entities_in_viewport)
    if(player.components.stats_component.level ~= Game.level_cap) then
        if(player.components.stats_component.experience >=
            Game.level_data[player.components.stats_component.level + 1].experience) then
            player.components.stats_component:level_up(Game, player)
        end
    end
end

function loot_system(player, entities, entities_in_viewport)
    if(player.components.score_update_component ~= nil) then
        player.components.stats_component:add_score(player, player.components.score_update_component.value)
        local treasure_chest_spawn_position = entities[player.components.score_update_component.entity_group][player.components.score_update_component.entity_full_name].components.position_component
        remove_entity(player.components.score_update_component.entity_group, player.components.score_update_component.entity_id)

        if(player.components.score_update_component.entity_group == "mobs") then
            local treasure_chest_drop_chance = get_random_number(1, 100)
            local treasure_chest_name = nil

            if(treasure_chest_drop_chance >= 90) then
                treasure_chest_name = "exquisite"
            elseif (treasure_chest_drop_chance >= 70 and treasure_chest_drop_chance <= 89) then
                treasure_chest_name = "ornate"
            elseif (treasure_chest_drop_chance >= 40 and treasure_chest_drop_chance <= 69) then
                treasure_chest_name = "common"
            elseif (treasure_chest_drop_chance >= 10 and treasure_chest_drop_chance <= 39) then
                treasure_chest_name = "ordinary"
            end

            if(treasure_chest_name ~= nil) then
                spawn_treasure_chest(treasure_chest_name, treasure_chest_spawn_position)
            end
        end

        remove_component("common", "player", "score_update_component")
    elseif (player.components.health_update_component ~= nil) then
        player.components.stats_component:add_health(player, player.components.health_update_component.value)
        remove_entity(player.components.health_update_component.entity_group, player.components.health_update_component.entity_id)
        remove_component("common", "player", "health_update_component")
    end
end

-- This gets called every 1 second
function tick_system(player, entities, entities_in_viewport)
    Game.health_recovery_timer = Game.health_recovery_timer + 1

    if(Game.health_recovery_timer == 2) then
        Game.health_recovery_timer = 0

        if(player.components.stats_component.health < player.components.stats_component.max_health) then
            player.components.tick_component:tick(Game, player)
            force_redraw_map()
        end
    end
end

function mob_movement_system(player, entities, entities_in_viewport)
    local move_chance = get_random_number(1, 100)

    if(move_chance <= 20) then
        for key, value in pairs(entities_in_viewport) do
            if(entities.mobs[key] ~= nil) then
                local adjacent_points = get_adjacent_points(entities.mobs[key].components.position_component.x, entities.mobs[key].components.position_component.y)
                local dir = get_random_key_from_table(adjacent_points)
                if not adjacent_points[dir].blocked and
                       adjacent_points[dir].x ~= player.components.position_component.x and
                       adjacent_points[dir].y ~= player.components.position_component.y
                then
                    entities.mobs[key].components.position_component = { x = adjacent_points[dir].x, y = adjacent_points[dir].y }
                end
            end
        end
    end
end