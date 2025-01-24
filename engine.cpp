#include "engine.h"

#include <ranges>
#include <map>
#include <queue>
#include <random>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <mpg123.h>
#include <SDL2/SDL_image.h>
#include <fmt/ranges.h>

std::string generate_uuid()
{
  boost::uuids::random_generator gen;
  const boost::uuids::uuid id = gen();
  return to_string(id);
}

int generate_random_int(const int min, const int max)
{
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(min, max);
  return dis(gen);
}

namespace roguely::level_generation
{
  int get_neighbor_wall_count(std::shared_ptr<boost::numeric::ublas::matrix<int>> map, int map_width, int map_height, int x, int y)
  {
    int wall_count = 0;

    for (int row = y - 1; row <= y + 1; row++)
    {
      for (int col = x - 1; col <= x + 1; col++)
      {
        if (row >= 1 && col >= 1 && row < map_height - 1 && col < map_width - 1)
        {
          if ((*map)(row, col) == 0)
            wall_count++;
        }
        else
        {
          wall_count++;
        }
      }
    }

    return wall_count;
  }

  void perform_cellular_automaton(std::shared_ptr<boost::numeric::ublas::matrix<int>> map, int map_width, int map_height, int passes)
  {
    for (int p = 0; p < passes; p++)
    {
      auto &temp_map = std::make_shared<boost::numeric::ublas::matrix<int>>() = map;

      for (int rows = 0; rows < map_height; rows++)
      {
        for (int columns = 0; columns < map_width; columns++)
        {
          auto neighbor_wall_count = get_neighbor_wall_count(temp_map, map_width, map_height, columns, rows);

          if (neighbor_wall_count > 4)
          {
            (*map)(rows, columns) = 0; // 0 = wall
          }
          else
            (*map)(rows, columns) = 1; // 1 = floor
        }
      }
    }
  }

  std::shared_ptr<boost::numeric::ublas::matrix<int>> init_cellular_automata(int map_width, int map_height)
  {
    auto map = std::make_shared<boost::numeric::ublas::matrix<int>>(map_height, map_width);

    for (int r = 0; r < map_height; ++r)
    {
      for (int c = 0; c < map_width; ++c)
      {
        auto z = std::rand() % 100 + 1;
        if (z > 48)
        {
          (*map)(r, c) = 1;
        }
        else
          (*map)(r, c) = 0;
      }
    }

    return map;
  }
}

namespace roguely::common
{
  int Text::load_font(const std::string &path, int ptsize)
  {
    font = TTF_OpenFont(path.c_str(), ptsize);

    if (!font)
    {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to load font: %s\n%s", path, TTF_GetError());
      return -1;
    }

    return 0;
  }

  Size Text::get_text_extents(const std::string &text)
  {
    int w{}, h{};

    if (TTF_SizeText(font, text.c_str(), &w, &h) == 0)
    {
      return {w, h};
    }

    return {};
  }

  void Text::draw_text(SDL_Renderer *renderer, int x, int y, const std::string &text)
  {
    draw_text(renderer, x, y, text, text_color);
  }

  void Text::draw_text(SDL_Renderer *renderer, int x, int y, const std::string &t, SDL_Color color)
  {
    if (t.size() <= 0)
      return;

    if (text != t)
    {
      text = t;
      SDL_DestroyTexture(text_texture);
      SDL_Surface *text_surface = TTF_RenderText_Blended(font, t.c_str(), color);
      text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
      text_rect = {x, y, text_surface->w, text_surface->h};
      SDL_FreeSurface(text_surface);
    }

    SDL_RenderCopy(renderer, text_texture, nullptr, &text_rect);
  }
}

namespace roguely::ecs
{
  std::string entity_group_name_to_string(roguely::ecs::EntityGroupName group_name)
  {
    auto gn = std::string(magic_enum::enum_name(group_name));
    to_lower(gn);
    return gn;
  }

  std::shared_ptr<EntityGroup> EntityManager::create_entity_group(const std::string &group_name)
  {
    auto entityGroup = std::make_shared<EntityGroup>();
    entityGroup->name = group_name;
    entityGroup->entities = std::make_shared<std::vector<std::shared_ptr<Entity>>>();
    entity_groups->emplace_back(entityGroup);
    return entityGroup;
  }

  void EntityManager::add_entity_to_group(const std::string &group_name, std::shared_ptr<Entity> e, sol::this_state s)
  {
    sol::state_view lua(s);
    auto group = get_entity_group(group_name);
    if (group == nullptr)
    {
      group = std::make_shared<EntityGroup>();
      group->name = group_name;
      group->entities = std::make_unique<std::vector<std::shared_ptr<Entity>>>();
      entity_groups->emplace_back(group);
    }
    group->entities->emplace_back(e);

    // Create Lua mapping (entity_group->entity->components)
    sol::table lua_entity_table;
    if (!lua_entities[group_name].valid())
    {
      lua_entity_table = lua.create_table();
    }
    else
      lua_entity_table = lua_entities[group_name];

    auto lua_component = e->find_first_component_by_type<roguely::components::LuaComponent>();

    if (lua_component != nullptr)
    {
      auto full_name = fmt::format("{}-{}", e->get_name(), e->get_id());
      lua_entity_table.set(full_name,
                           lua.create_table_with(
                               "id", e->get_id(),
                               "name", e->get_name(),
                               "full_name", full_name,
                               "components", lua_component->get_properties()));
    }

    lua_entities.set(group_name, lua_entity_table);
  }

  std::shared_ptr<Entity> EntityManager::create_entity_in_group(const std::string &group_name, const std::string &entity_name)
  {
    auto entity = std::make_shared<Entity>(entity_name);
    auto entity_group = get_entity_group(group_name);
    if (entity_group != nullptr)
    {
      entity_group->entities->emplace_back(entity);
      return entity;
    }
    return nullptr;
  }

  void EntityManager::remove_entity(const std::string &entity_group_name, const std::string &entity_id)
  {
    auto entity_group = get_entity_group(entity_group_name);

    if (entity_group != nullptr)
    {
      auto entity_to_remove = std::find_if(entity_group->entities->begin(),
                                           entity_group->entities->end(),
                                           [&](std::shared_ptr<Entity> e)
                                           {
                                             return (e->get_id() == entity_id) ? true : false;
                                           });

      if (entity_to_remove != entity_group->entities->end())
      {
        std::string full_name = fmt::format("{}-{}", (*entity_to_remove)->get_name(), (*entity_to_remove)->get_id());

        // fmt::println("Removing entity: {}", full_name);

        sol::table entity_group_table = lua_entities[entity_group_name];
        entity_group_table.set(full_name, sol::nil);

        // fmt::println("Checking entity is valid: {}", entity_group_table[full_name].valid());

        entity_group->entities->erase(entity_to_remove);
      }
    }
  }

  std::shared_ptr<EntityGroup> EntityManager::get_entity_group(const std::string &group_name)
  {
    auto group = std::ranges::find_if(*entity_groups,
                                      [&](const std::shared_ptr<EntityGroup> &eg)
                                      {
                                        return eg->name == group_name;
                                      });

    if (group != entity_groups->end())
    {
      return *group;
    }

    return nullptr;
  }

  std::shared_ptr<std::vector<std::shared_ptr<Entity>>> EntityManager::get_entities_in_group(const std::string &group_name)
  {
    auto entity_group = get_entity_group(group_name);

    if (entity_group != nullptr)
    {
      return entity_group->entities;
    }

    return nullptr;
  }

  std::string EntityManager::get_entity_id_by_name(const std::string &group_name, const std::string &entity_name)
  {
    auto entity = EntityManager::get_entity_by_name(group_name, entity_name);
    if (entity != nullptr)
    {
      return entity->get_id();
    }
    return "";
  }

  std::shared_ptr<Entity> EntityManager::get_entity_by_name(const std::string &entity_group, const std::string &entity_name)
  {
    return find_entity(entity_group, [&](const std::shared_ptr<Entity> &e)
                       { return e->get_name() == entity_name; });
  }

  std::shared_ptr<Entity> EntityManager::get_entity_by_id(const std::string &entity_group, const std::string &entity_id)
  {
    return find_entity(entity_group, [&](const std::shared_ptr<Entity> &e)
                       { return e->get_id() == entity_id; });
  }

  std::shared_ptr<std::vector<std::shared_ptr<Entity>>> EntityManager::find_entities_in_group(const std::string &entity_group, std::function<bool(std::shared_ptr<Entity>)> predicate)
  {
    auto entity_group_ptr = get_entity_group(entity_group);
    auto entities = std::make_shared<std::vector<std::shared_ptr<Entity>>>();

    if (entity_group_ptr != nullptr)
    {
      for (const auto &e : *entity_group_ptr->entities)
      {
        if (predicate(e))
        {
          entities->emplace_back(e);
        }
      }

      return entities;
    }

    return nullptr;
  }

  std::shared_ptr<Entity> EntityManager::find_entity(const std::string &entity_group, std::function<bool(std::shared_ptr<Entity>)> predicate)
  {
    auto entity_group_ptr = get_entity_group(entity_group);

    if (entity_group_ptr != nullptr)
    {
      auto entity = std::find_if(entity_group_ptr->entities->begin(), entity_group_ptr->entities->end(),
                                 [&](const std::shared_ptr<Entity> &e)
                                 {
                                   return predicate(e);
                                 });

      if (entity != entity_group_ptr->entities->end())
      {
        return *entity;
      }
    }

    return nullptr;
  }

  bool EntityManager::lua_entities_for_each(std::function<bool(sol::table)> predicate)
  {
    bool result = false;

    for (const auto &eg : *entity_groups)
    {
      for (const auto &e : *eg->entities)
      {
        auto lua_component = e->find_first_component_by_type<roguely::components::LuaComponent>();

        if (lua_component != nullptr)
        {
          auto lua_components_table = lua_component->get_properties();

          if (lua_components_table.valid())
          {
            result = predicate(lua_components_table);
          }
        }
      }
    }

    return result;
  }

  bool EntityManager::lua_is_point_unique(roguely::common::Point point)
  {
    auto result = true;

    for (const auto &eg : *entity_groups)
    {
      for (const auto &e : *eg->entities)
      {
        auto lua_component = e->find_first_component_by_type<roguely::components::LuaComponent>();

        if (lua_component != nullptr)
        {
          auto lua_components_table = lua_component->get_properties();

          if (lua_components_table.valid())
          {
            auto position_component = lua_components_table["position_component"];
            if (position_component.valid())
            {
              int x = position_component["x"];
              int y = position_component["y"];

              if (x == point.x && y == point.y)
              {
                result = false;
                break;
              }
            }
            // else
            // {
            //   fmt::println("position_component is not valid: {}", e->get_name());
            // }
          }
        }
      }
    }

    return result;
  }

  void EntityManager::lua_for_each_overlapping_point(const std::string &entity_name, int x, int y, sol::function point_callback)
  {
    for (const auto &eg : *entity_groups)
    {
      for (const auto &e : *eg->entities)
      {
        auto lua_component = e->find_first_component_by_type<roguely::components::LuaComponent>();

        if (e->get_name() != entity_name && lua_component != nullptr)
        {
          auto lua_components_table = lua_component->get_properties();

          if (lua_components_table.valid())
          {
            auto position_component = lua_components_table["position_component"];
            if (position_component.valid())
            {
              int pc_x = position_component["x"];
              int pc_y = position_component["y"];

              if (pc_x == x && pc_y == y)
              {
                // fmt::println("found overlapping point: Player({}, {}) == Entity({}, {})", x, y, pc_x, pc_y);
                auto point_callback_result = point_callback(fmt::format("{}-{}", e->get_name(), e->get_id()), e->get_name(), lua_components_table);
                if (!point_callback_result.valid())
                {
                  sol::error err = point_callback_result;
                  fmt::println("Lua script error: {}", err.what());
                }
              }
            }
          }
        }
      }
    }
  }

  sol::table EntityManager::get_lua_blocked_points(const std::string &entity_group, int x, int y, const std::string &direction, sol::this_state s)
  {
    sol::state_view lua(s);
    auto eg = get_entity_group(entity_group);
    sol::table result = lua.create_table();

    for (const auto &e : *eg->entities)
    {
      auto lua_component = e->find_first_component_by_type<roguely::components::LuaComponent>();

      if (lua_component != nullptr)
      {
        auto lua_components_table = lua_component->get_properties();

        if (lua_components_table.valid())
        {
          auto position_component = lua_components_table["position_component"];
          if (position_component.valid())
          {
            bool is_blocked = false;

            int entity_x = position_component["x"];
            int entity_y = position_component["y"];

            int up_position_y = y - 1;
            int down_position_y = y + 1;
            int left_position_x = x - 1;
            int right_position_x = x + 1;

            // UP
            if (direction == "up" && entity_x == x && up_position_y == entity_y)
            {
              is_blocked = true;
            }
            // DOWN
            else if (direction == "down" && entity_x == x && entity_y == down_position_y)
            {
              is_blocked = true;
            }
            // LEFT
            else if (direction == "left" && entity_x == left_position_x && entity_y == y)
            {
              is_blocked = true;
            }
            // RIGHT
            else if (direction == "right" && entity_x == right_position_x && entity_y == y)
            {
              is_blocked = true;
            }

            if (is_blocked)
            {
              // fmt::println("found overlapping point: Player({}, {}) == Entity({}, {})", x, y, entity_x, entity_y);

              result.set("entity_name", e->get_name());
              result.set("entity_full_name", fmt::format("{}-{}", e->get_name(), e->get_id()));
              result.set("entity_position", lua.create_table_with("x", entity_x, "y", entity_y));
              result.set("direction", direction);
              break;
            }
          }
        }
      }
    }

    return result;
  }

  sol::table EntityManager::get_lua_entities_in_viewport(std::function<bool(int x, int y)> predicate, sol::this_state s)
  {
    // loop through all of the entity groups and find the entities that are in the viewport
    // return a table of entities that are in the viewport
    sol::state_view lua(s);
    sol::table result = lua.create_table();

    for (const auto &eg : *entity_groups)
    {
      for (const auto &e : *eg->entities)
      {
        auto lua_component = e->find_first_component_by_type<roguely::components::LuaComponent>();

        if (lua_component != nullptr)
        {
          auto lua_components_table = lua_component->get_properties();

          if (lua_components_table.valid())
          {
            auto position_component = lua_components_table["position_component"];
            if (position_component.valid())
            {
              auto x = position_component["x"];
              auto y = position_component["y"];

              if (predicate(x, y))
              {
                result.set(fmt::format("{}-{}", e->get_name(), e->get_id()),
                           lua.create_table_with(
                               "group_name", eg->name,
                               "name", e->get_name(),
                               "full_name", fmt::format("{}-{}", e->get_name(), e->get_id())));
              }
            }
          }
        }
      }
    }

    return result;
  }
}

namespace roguely::sprites
{
  SpriteSheet::SpriteSheet(SDL_Renderer *renderer, const std::string &n, const std::string &p, int sw, int sh, int sf)
  {
    path = p;
    name = n;
    sprite_width = sw;
    sprite_height = sh;

    if (sf <= 0)
      sf = 1;

    scale_factor = sf;

    // fmt::println("loading spritesheet: {}", path);
    // fmt::println("sprite width: {} | sprite height: {}", sprite_width, sprite_height);
    // fmt::println("scale factor: {}", scale_factor);

    auto tileset = IMG_Load(p.c_str());
    spritesheet_texture = SDL_CreateTextureFromSurface(renderer, tileset);
    int total_sprites_on_sheet = tileset->w / sw * tileset->h / sh;
    sprites = std::make_unique<std::vector<std::shared_ptr<SDL_Rect>>>(0);
    // fmt::println("total sprites on sheet: {}", total_sprites_on_sheet);

    SDL_GetTextureColorMod(spritesheet_texture, &o_red, &o_green, &o_blue);

    for (int y = 0; y < total_sprites_on_sheet / (sw + sh); y++)
    {
      for (int x = 0; x < total_sprites_on_sheet / (sw + sh); x++)
      {
        SDL_Rect rect = {x * sw, y * sh, sw, sh};
        sprites->emplace_back(std::make_shared<SDL_Rect>(rect));
      }
    }

    sprites->resize(total_sprites_on_sheet);

    SDL_FreeSurface(tileset);
  }

  void SpriteSheet::draw_sprite(SDL_Renderer *renderer, int sprite_id, int x, int y)
  {
    draw_sprite(renderer, sprite_id, x, y, scale_factor);
  }

  void SpriteSheet::draw_sprite(SDL_Renderer *renderer, int sprite_id, int x, int y, int scale_factor)
  {
    if (sprite_id < 0 || sprite_id > sprites->capacity())
    {
      fmt::println("sprite id out of range: {}", sprite_id);
      return;
    }

    int width = sprite_width;
    int height = sprite_height;

    if (scale_factor > 0)
    {
      width = sprite_width * scale_factor;
      height = sprite_height * scale_factor;
    }

    SDL_Rect dest = {x, y, width, height};
    auto &sprite_rect = sprites->at(sprite_id);
    SDL_RenderCopy(renderer, spritesheet_texture, &(*sprite_rect), &dest);
  }

  void SpriteSheet::draw_sprite_sheet(SDL_Renderer *renderer, int x, int y)
  {
    int col = 0;
    int row_height = 0;

    // fmt::println("scale_factor: {}", scale_factor);

    for (int i = 0; i < sprites->size(); i++)
    {
      // fmt::println("col * sprite_width = {}", col * sprite_width);
      // fmt::println("col * (sprite_width * scale_factor) = {}", col * (sprite_width * scale_factor));

      draw_sprite(renderer, i, x + (col * (sprite_width * scale_factor)), y + row_height);
      col++;

      if ((i + 1) % 16 == 0)
      {
        row_height += sprite_height * scale_factor;
        col = 0;
      }
    }
  }

  sol::table SpriteSheet::get_sprites_as_lua_table(sol::this_state s)
  {
    sol::state_view lua(s);
    sol::table sprites_table = lua.create_table();

    // std::cout << "getting info for: " << name << std::endl;
    // std::cout << "number of sprites: " << sprites->size() << std::endl;

    for (std::size_t i = 0, sp = sprites->size(); i != sp; ++i)
    {
      auto &sprite_rect = sprites->at(i);

      sol::table rect_table = lua.create_table();

      rect_table.set("x", sprite_rect->x);
      rect_table.set("y", sprite_rect->y);
      rect_table.set("w", sprite_rect->w);
      rect_table.set("h", sprite_rect->h);

      sprites_table.set(i, rect_table);

      // std::cout << i << " - " <<
      //		" x = " << sprite_rect->x <<
      //		" y = " << sprite_rect->x <<
      //		" w = " << sprite_rect->w <<
      //		" h = " << sprite_rect->h << std::endl;
    }

    return sprites_table;
  }
}

namespace roguely::map
{
  void Map::draw_map(SDL_Renderer *renderer, roguely::common::Dimension dimensions, std::shared_ptr<roguely::sprites::SpriteSheet> sprite_sheet,
                     std::function<void(int, int, int, int, int, int, int)> draw_hook)
  {
    int scale_factor = sprite_sheet->get_scale_factor();
    int sprite_width = sprite_sheet->get_sprite_width();
    int sprite_height = sprite_sheet->get_sprite_height();

    int texture_width = dimensions.size.width * (sprite_width * scale_factor);
    int texture_height = dimensions.size.height * (sprite_height * scale_factor);

    if (!current_map_segment_dimension.eq(dimensions))
    {
      current_map_segment_dimension = dimensions;

      if (current_map_segment_texture != nullptr)
        SDL_DestroyTexture(current_map_segment_texture);

      current_map_segment_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, texture_width, texture_height);
      SDL_SetRenderTarget(renderer, current_map_segment_texture);
      SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
      SDL_RenderClear(renderer);

      // fmt::println("sprite width: {} | sprite height: {}", sprite_width, sprite_height);
      // fmt::println("dimensions.point.x: {} | dimensions.point.y: {}", dimensions.point.x, dimensions.point.y);
      // fmt::println("dimensions.size.width: {} | dimensions.size.height: {}", dimensions.size.width, dimensions.size.height);
      // fmt::println("map width: {} | map height: {}", map->size2(), map->size1());
      // fmt::println("total sprites: {}", sprite_sheet->get_size_of_sprites());

      for (int rows = dimensions.point.y; rows < dimensions.size.height; rows++)
      {
        for (int cols = dimensions.point.x; cols < dimensions.size.width; cols++)
        {
          int dx = (cols * sprite_width * scale_factor) -
                   (dimensions.point.x * sprite_width * scale_factor);
          int dy = (rows * sprite_height * scale_factor) -
                   (dimensions.point.y * sprite_height * scale_factor);

          int cell_id = (*map)(rows, cols);

          if (draw_hook != nullptr)
          {
            // rows, cols = map Y, X
            // dx, dy = world X, Y
            auto light_cell = (*light_map)(rows, cols);
            // fmt::println("light cell: {}", light_cell);
            draw_hook(rows, cols, dx, dy, cell_id, light_cell, scale_factor);
          }
        }
      }
    }

    SDL_SetRenderTarget(renderer, NULL);
    SDL_Rect destination = {0, 0, texture_width, texture_height};
    SDL_RenderCopy(renderer, current_map_segment_texture, NULL, &destination);
  }

  void Map::draw_map(SDL_Renderer *renderer, roguely::common::Dimension dimensions, int dest_x, int dest_y, int a, std::function<void(int, int, int)> draw_hook)
  {
    if (!current_full_map_dimension.eq(dimensions))
    {
      current_full_map_dimension = dimensions;

      if (current_full_map_texture != nullptr)
        SDL_DestroyTexture(current_full_map_texture);

      current_full_map_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, width, height);
      SDL_SetTextureBlendMode(current_full_map_texture, SDL_BLENDMODE_BLEND);
      SDL_SetRenderTarget(renderer, current_full_map_texture);
      SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
      SDL_SetTextureAlphaMod(current_full_map_texture, a);
      SDL_RenderClear(renderer);

      for (int rows = 0; rows < height; rows++)
      {
        for (int cols = 0; cols < width; cols++)
        {
          int cell_id = (*map)(rows, cols);

          if (draw_hook != nullptr)
          {
            draw_hook(rows, cols, cell_id);
          }
        }
      }
    }

    SDL_SetRenderTarget(renderer, NULL);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_Rect destination = {dest_x, dest_y, width, height};
    SDL_RenderCopy(renderer, current_full_map_texture, NULL, &destination);
  }

  void Map::calculate_field_of_view(roguely::common::Dimension dimensions)
  {
    light_map = std::make_shared<boost::numeric::ublas::matrix<int>>(height, width, 0);

    // Iterate through all angles in the 360-degree field of view
    for (int angle = 0; angle < 360; angle += 1)
    {
      float radians = angle * 0.01745329f; // Conversion factor from degrees to radians
      float dx = std::cos(radians);
      float dy = std::sin(radians);

      float newX = dimensions.supplimental_point.x + dx;
      float newY = dimensions.supplimental_point.y + dy;

      // Keep expanding in the current direction until reaching a wall or map boundary
      while (newX >= 0 && newX < width && newY >= 0 && newY < height)
      {
        // Mark the cell as visible
        (*light_map)((int)newY, (int)newX) = 1;

        // Stop expanding if a wall is encountered
        if ((*map)((int)newY, (int)newX) == 0)
          break;

        // Move to the next cell in the current direction
        newX += dx;
        newY += dy;
      }
    }
  }

  roguely::common::Point Map::get_random_point(std::set<int> off_limit_sprites_ids)
  {
    if (width == 0 || height == 0)
    {
      throw std::runtime_error("Empty map");
    }

    if (off_limit_sprites_ids.empty())
    {
      return roguely::common::Point{generate_random_int(0, height - 1), generate_random_int(0, width - 1)};
    }

    size_t maxAttempts = height * width;
    size_t attempts = 0;

    while (attempts < maxAttempts)
    {
      int row = generate_random_int(0, height - 1);
      int col = generate_random_int(0, width - 1);

      if (off_limit_sprites_ids.find((*map)(row, col)) == off_limit_sprites_ids.end())
      {
        // fmt::println("Found random point: ({},{}) = {}", row, col, (*map)(row, col));
        return roguely::common::Point{col, row};
      }

      attempts++;
    }

    throw std::runtime_error("Unable to find a random point in map");
  }

  std::vector<std::pair<int, int>> AStar::FindPath(const boost::numeric::ublas::matrix<int> &grid, int start_row, int start_col, int goal_row, int goal_col)
  {
    // Check if the starting position is a zero
    if (grid(start_col, start_row) != 0)
      return {};

    // Store grid size
    int grid_rows = static_cast<int>(grid.size1());
    int grid_cols = static_cast<int>(grid.size2());

    // Create a priority queue to store the open list
    std::priority_queue<pq_entry, std::vector<pq_entry>, std::greater<pq_entry>> open_list;
    // Create a matrix to store the cost of reaching each cell
    boost::numeric::ublas::matrix<int> cost(grid_rows, grid_cols, INT_MAX);
    // Create a matrix to store the parent of each cell
    boost::numeric::ublas::matrix<std::pair<int, int>> parent(grid_rows, grid_cols);
    // Initialize the cost of the start cell to 0
    cost(start_row, start_col) = 0;
    // Add the start cell to the open list with a priority of 0
    open_list.emplace(0, std::make_pair(start_row, start_col));

    while (!open_list.empty())
    {
      // Get the cell with the lowest cost from the open list
      auto [_, top_pair] = open_list.top();
      auto [x, y] = top_pair;
      open_list.pop();

      // Check if we have reached the goal cell
      if (x == goal_col && y == goal_row)
      {
        // Reconstruct the path from the goal to the start
        std::vector<std::pair<int, int>> path;
        while (x != start_row || y != start_col)
        {
          path.emplace_back(x, y);
          std::tie(x, y) = parent(x, y);
        }
        path.emplace_back(start_row, start_col);
        std::reverse(path.begin(), path.end());
        return path;
      }

      // Explore the neighbors of the current cell
      for (int i = 0; i < 4; i++)
      {
        int nx = x + dx[i];
        int ny = y + dy[i];

        // Check if the neighbor is within the grid bounds
        if (nx >= 0 && nx < grid_rows && ny >= 0 && ny < grid_cols && grid(nx, ny) == 0)
        {
          // Calculate the cost of reaching the neighbor
          int new_cost = cost(x, y) + 1;

          // Check if the new cost is lower than the current cost
          if (new_cost < cost(nx, ny))
          {
            // Update the cost and parent of the neighbor
            cost(nx, ny) = new_cost;
            parent(nx, ny) = std::make_pair(x, y);

            // Calculate the priority of the neighbor (cost + heuristic)
            // Add the neighbor to the open list
            open_list.emplace(new_cost + heuristic(nx, ny, goal_col, goal_row), std::make_pair(nx, ny));
          }
        }
      }
    }

    return {};
  }
}

namespace roguely::engine
{
  Engine::Engine()
  {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    Mix_OpenAudio(44100, AUDIO_S16SYS, 2, 4096);
    Mix_Volume(-1, 3);
    Mix_VolumeMusic(5);

    entity_manager = std::make_unique<roguely::ecs::EntityManager>(lua.lua_state());
    maps = std::make_unique<std::vector<std::shared_ptr<roguely::map::Map>>>();
    systems = std::make_unique<std::unordered_map<std::string, sol::function>>();
    texts = std::make_unique<std::unordered_map<std::string, std::shared_ptr<roguely::common::Text>>>();
  }

  int Engine::init_sdl(sol::table game_config, sol::this_state s)
  {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
    {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to initialize SDL: %s", SDL_GetError());
      return -1;
    }

    if (IMG_Init(IMG_INIT_PNG) < 0)
    {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to initialize SDL2 IMG: %s", IMG_GetError());
      return -1;
    }

    if (TTF_Init() < 0)
    {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to initialize SDL_ttf: %s", TTF_GetError());
      return -1;
    }

    if (Mix_Init(MIX_INIT_MP3) == 0)
    {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to initialize SDL2 mixer: %s", Mix_GetError());
      return -1;
    }

    std::string window_title = game_config["window_title"];
    std::string window_icon_path = game_config["window_icon_path"];
    int window_width = game_config["window_width"];
    int window_height = game_config["window_height"];
    int spritesheet_sprite_width = game_config["spritesheet_sprite_width"];
    int spritesheet_sprite_height = game_config["spritesheet_sprite_height"];
    int spritesheet_sprite_scale_factor = game_config["spritesheet_sprite_scale_factor"];
    VIEW_PORT_WIDTH = window_width / (spritesheet_sprite_width * spritesheet_sprite_scale_factor);
    VIEW_PORT_HEIGHT = window_height / (spritesheet_sprite_height * spritesheet_sprite_scale_factor);
    current_dimension = {0, 0, VIEW_PORT_WIDTH, VIEW_PORT_HEIGHT};
    game_config["viewport_width"] = VIEW_PORT_WIDTH;
    game_config["viewport_height"] = VIEW_PORT_HEIGHT;
    game_config["keycodes"] = lua.create_table_with(
        1073741906, "up",
        1073741905, "down",
        1073741904, "left",
        1073741903, "right",
        119, "w",
        97, "a",
        115, "s",
        100, "d",
        32, "space");

    window = SDL_CreateWindow(window_title.c_str(),
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                              window_width, window_height,
                              SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (!window)
    {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create SDL window: %s", SDL_GetError());
      SDL_Quit();
      return 1;
    }

    SDL_Surface *window_icon_surface = IMG_Load(window_icon_path.c_str());
    SDL_SetWindowIcon(window, window_icon_surface);
    SDL_FreeSurface(window_icon_surface);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);

    if (renderer == NULL)
    {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL could not create renderer: %s", SDL_GetError());
      SDL_DestroyWindow(window);
      SDL_Quit();
      return 1;
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    // FIXME: Need to create a way for user defined Text objects
    // std::string font_path = game_config["font_path"];
    // text_medium = std::make_unique<roguely::common::Text>();
    // text_medium->load_font(font_path, 32);

    // Load the spritesheet (FIXME: add ability to load more than one spritesheet)
    sprite_sheets = std::make_unique<std::unordered_map<std::string, std::shared_ptr<roguely::sprites::SpriteSheet>>>();
    sprite_sheets->emplace(game_config["spritesheet_name"],
                           std::make_shared<roguely::sprites::SpriteSheet>(renderer,
                                                                           game_config["spritesheet_name"],
                                                                           game_config["spritesheet_path"],
                                                                           game_config["spritesheet_sprite_width"],
                                                                           game_config["spritesheet_sprite_height"],
                                                                           game_config["spritesheet_sprite_scale_factor"]));

    // Initialize sounds
    if (game_config["sounds"].valid() && game_config["sounds"].get_type() == sol::type::table)
    {
      sounds = std::make_unique<std::vector<std::shared_ptr<roguely::common::Sound>>>();

      sol::table sound_table = game_config["sounds"];

      for (auto &sound : sound_table)
      {
        if (sound.first.get_type() == sol::type::string &&
            sound.second.get_type() == sol::type::string)
        {
          // fmt::println("loading sound: {}", sound.first.as<std::string>());
          // fmt::println("sound path: {}", sound.second.as<std::string>());

          auto sound_name = sound.first.as<std::string>();
          auto sound_path = sound.second.as<std::string>();

          // check to see if the sound file exists
          if (!std::filesystem::exists(sound_path))
          {
            fmt::println("sound file does not exist: {}", sound_path);
          }
          else
          {
            roguely::common::Sound s{sound_name, Mix_LoadWAV(sound_path.c_str())};
            sounds->emplace_back(std::make_shared<roguely::common::Sound>(s));
          }
        }
      }
    }

    if (game_config["soundtrack_path"].valid() && game_config["soundtrack_path"].get_type() == sol::type::string)
    {
      std::string soundtrack_path = game_config["soundtrack_path"];

      soundtrack = Mix_LoadMUS(soundtrack_path.c_str());
      Mix_PlayMusic(soundtrack, 1);
    }

    return 0;
  }

  void Engine::tear_down_sdl()
  {
    if (soundtrack != nullptr)
    {
      Mix_FreeMusic(soundtrack);
    }

    for (auto &s : *sounds)
    {
      Mix_FreeChunk(s->sound);
    }

    sprite_sheets.reset();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    Mix_Quit();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
  }

  int Engine::game_loop()
  {
    lua.open_libraries(sol::lib::base,
                       sol::lib::math,
                       sol::lib::debug,
                       sol::lib::string);

    std::string roguely_script = "roguely.lua";
    if (!std::filesystem::exists(roguely_script))
    {
      fmt::println("'roguely.lua' does not exist.");
      return -1;
    }

    auto game_script = lua.safe_script_file(roguely_script);

    if (!game_script.valid())
    {
      sol::error err = game_script;
      fmt::println("Lua script error: {}", err.what());
      return -1;
    }

    auto game_config = lua.get<sol::table>("Game");

    if (!(game_config.valid() && check_game_config(game_config, lua.lua_state())))
    {
      fmt::println("game script does not define the 'Game' configuration table.");
      return -1;
    }

    if (init_sdl(game_config, lua.lua_state()) < 0)
      return -1;

    setup_lua_api(lua.lua_state());

    if (check_if_lua_function_defined(lua.lua_state(), "_init"))
    {
      auto init_result = lua["_init"]();
      if (!init_result.valid())
      {
        sol::error err = init_result;
        fmt::println("Lua script error: {}", err.what());
      }
    }

    SDL_Event e;
    bool quit = false;
    const int fps = 6;
    const int frame_delay = 1000 / fps;

    Uint32 last_update_time = SDL_GetTicks();
    const Uint32 update_interval = 1000;

    Uint32 frame_start;
    int frame_time;

    while (!quit)
    {
      frame_start = SDL_GetTicks();

      // handle events
      while (SDL_PollEvent(&e))
      {
        if (e.type == SDL_QUIT)
        {
          quit = true;
        }
        else if (e.type == SDL_KEYDOWN)
        {
          if (systems->contains("keyboard_input_system"))
          {
            // FIXME: Fix hard coded entity group and entity name for PLAYER
            auto keyboard_input_system_result = (*systems)["keyboard_input_system"](e.key.keysym.sym,
                                                                                    entity_manager->get_lua_entity("common", "player"),
                                                                                    entity_manager->get_lua_entities(),
                                                                                    entity_manager->get_lua_entities_in_viewport(
                                                                                        [&](int x, int y)
                                                                                        {
                                                                                          return is_within_viewport(x, y);
                                                                                        },
                                                                                        lua.lua_state()));
            if (!keyboard_input_system_result.valid())
            {
              sol::error err = keyboard_input_system_result;
              fmt::println("Lua script error: {}", err.what());
            }
          }
        }
      }

      for (auto &system : *systems)
      {
        if (system.first != "tick_system" && system.first != "keyboard_input_system" &&
            system.first != "render_system")
        {
          auto system_result = system.second(entity_manager->get_lua_entity("common", "player"),
                                             entity_manager->get_lua_entities(),
                                             entity_manager->get_lua_entities_in_viewport(
                                                 [&](int x, int y)
                                                 {
                                                   return is_within_viewport(x, y);
                                                 },
                                                 lua.lua_state()));
          if (!system_result.valid())
          {
            sol::error err = system_result;
            fmt::println("Lua script error: {}", err.what());
            return -1;
          }
        }
      }

      Uint32 current_time = SDL_GetTicks();
      if (current_time - last_update_time >= update_interval)
      {
        if (systems->contains("tick_system"))
        {
          auto tick_system_result = (*systems)["tick_system"](entity_manager->get_lua_entity("common", "player"),
                                                              entity_manager->get_lua_entities(),
                                                              entity_manager->get_lua_entities_in_viewport(
                                                                  [&](int x, int y)
                                                                  {
                                                                    return is_within_viewport(x, y);
                                                                  },
                                                                  lua.lua_state()));
          if (!tick_system_result.valid())
          {
            sol::error err = tick_system_result;
            fmt::println("Lua script error: {}", err.what());
          }
        }
        last_update_time = current_time;
      }

      SDL_RenderClear(renderer);

      // Calculate delta time
      Uint32 current_frame_time = SDL_GetTicks();
      float delta_time = (current_frame_time - frame_start) / 1000.0f;

      // Call render
      if (systems->contains("render_system"))
      {
        auto render_system_result = (*systems)["render_system"](delta_time,
                                                                entity_manager->get_lua_entity("common", "player"),
                                                                entity_manager->get_lua_entities(),
                                                                entity_manager->get_lua_entities_in_viewport(
                                                                    [&](int x, int y)
                                                                    {
                                                                      return is_within_viewport(x, y);
                                                                    },
                                                                    lua.lua_state()));
        if (!render_system_result.valid())
        {
          sol::error err = render_system_result;
          fmt::println("Lua script error: {}", err.what());
        }
      }

      SDL_RenderPresent(renderer);

      // limit frame rate
      frame_time = SDL_GetTicks() - frame_start;
      if (frame_delay > frame_time)
      {
        SDL_Delay(frame_delay - frame_time);
      }
    }

    tear_down_sdl();

    return 0;
  }

  bool Engine::check_game_config(sol::table game_config, sol::this_state s)
  {
    bool result = true;

    auto title = game_config["window_title"];
    auto window_width = game_config["window_width"];
    auto window_height = game_config["window_height"];
    auto window_icon_path = game_config["window_icon_path"];
    auto font_path = game_config["font_path"];
    auto spritesheet_name = game_config["spritesheet_name"];
    auto spritesheet_path = game_config["spritesheet_path"];
    auto spritesheet_sprite_width = game_config["spritesheet_sprite_width"];
    auto spritesheet_sprite_height = game_config["spritesheet_sprite_height"];
    auto spritesheet_sprite_scale_factor = game_config["spritesheet_sprite_scale_factor"];
    auto sounds = game_config["sounds"];

    if (!(title.valid() && title.get_type() == sol::type::string))
      return false;
    if (!(window_width.valid() && window_width.get_type() == sol::type::number))
      return false;
    if (!(window_height.valid() && window_height.get_type() == sol::type::number))
      return false;
    if (!(font_path.valid() && font_path.get_type() == sol::type::string))
      return false;
    if (!(spritesheet_path.valid() && spritesheet_path.get_type() == sol::type::string))
      return false;
    if (!(sounds.valid() && sounds.get_type() == sol::type::table))
      return false;
    if (!(window_icon_path.valid() && window_icon_path.get_type() == sol::type::string))
      return false;
    if (!(spritesheet_name.valid() && spritesheet_name.get_type() == sol::type::string))
      return false;
    if (!(spritesheet_sprite_width.valid() && spritesheet_sprite_width.get_type() == sol::type::number))
      return false;
    if (!(spritesheet_sprite_height.valid() && spritesheet_sprite_height.get_type() == sol::type::number))
      return false;
    if (!(spritesheet_sprite_scale_factor.valid() && spritesheet_sprite_scale_factor.get_type() == sol::type::number))
      return false;

    return result;
  }

  void Engine::draw_text(const std::string &t, int x, int y)
  {
    draw_text(t, x, y, 255, 255, 255, 255);
  }

  void Engine::draw_text(const std::string &t, int x, int y, int r, int g, int b, int a)
  {
    if (!(t.length() > 0))
      return;

    if (!default_font.expired())
    {
      SDL_Color text_color = {(Uint8)r, (Uint8)g, (Uint8)b, (Uint8)a};
      default_font.lock()->draw_text(renderer, x, y, t, text_color);
    }
  }

  void Engine::draw_sprite(const std::string &spritesheet_name, int sprite_id, int x, int y, int scale_factor)
  {
    if (sprite_sheets->contains(spritesheet_name))
    {
      (*sprite_sheets)[spritesheet_name]->draw_sprite(renderer, sprite_id, x, y, scale_factor);
    }
  }

  void Engine::set_draw_color(SDL_Renderer *renderer, int r, int g, int b, int a)
  {
    SDL_SetRenderDrawColor(renderer, r, g, b, a);
  }

  void Engine::draw_point(SDL_Renderer *renderer, int x, int y)
  {
    SDL_RenderDrawPoint(renderer, x, y);
  }

  void Engine::draw_rect(SDL_Renderer *renderer, int x, int y, int w, int h)
  {
    SDL_Rect r = {x, y, w, h};
    SDL_RenderDrawRect(renderer, &r);
  }

  void Engine::draw_filled_rect(SDL_Renderer *renderer, int x, int y, int w, int h)
  {
    SDL_Rect r = {x, y, w, h};
    SDL_RenderFillRect(renderer, &r);
  }

  void Engine::draw_filled_rect_with_color(SDL_Renderer *renderer, int x, int y, int w, int h, int r, int g, int b, int a)
  {
    SDL_Rect rect = {x, y, w, h};
    SDL_SetRenderDrawColor(renderer, r, g, b, a);
    SDL_RenderFillRect(renderer, &rect);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  }

  void Engine::draw_graphic(SDL_Renderer *renderer, const std::string &path, int window_width, int x, int y, bool centered, int scale_factor)
  {
    if (!std::filesystem::exists(path))
    {
      fmt::println("graphic file does not exist: {}", path);
      return;
    }

    auto graphic = IMG_Load(path.c_str());
    auto graphic_texture = SDL_CreateTextureFromSurface(renderer, graphic);

    SDL_Rect src = {0, 0, graphic->w, graphic->h};
    SDL_Rect dest = {x, y, graphic->w, graphic->h};

    if (scale_factor > 0)
    {
      if (centered)
        dest = {((window_width / (2 + (int)scale_factor)) - (graphic->w / 2)), y, graphic->w, graphic->h};

      SDL_RenderSetScale(renderer, (float)scale_factor, (float)scale_factor);
      SDL_RenderCopy(renderer, graphic_texture, &src, &dest);
      SDL_RenderSetScale(renderer, 1, 1);
    }
    else
    {
      if (centered)
        dest = {((window_width / 2) - (graphic->w / 2)), y, graphic->w, graphic->h};

      SDL_RenderCopy(renderer, graphic_texture, &src, &dest);
    }

    SDL_FreeSurface(graphic);
    SDL_DestroyTexture(graphic_texture);
  }

  void Engine::play_sound(const std::string &name)
  {
    if (!(name.length() > 0))
      return;

    auto sound = std::find_if(sounds->begin(), sounds->end(),
                              [&](const auto &s)
                              {
                                return s->name == name;
                              });

    if (sound != sounds->end())
    {
      (*sound)->play();
    }
  }

  std::shared_ptr<roguely::map::Map> Engine::generate_map(const std::string &name, int map_width, int map_height)
  {
    // fmt::println("generating map: {}", name);

    auto map = roguely::level_generation::init_cellular_automata(map_width, map_height);
    roguely::level_generation::perform_cellular_automaton(map, map_width, map_height, 10);

    // auto map = std::make_shared<boost::numeric::ublas::matrix<int>>(map_height, map_width, 1);

    return std::make_shared<roguely::map::Map>(name, map_width, map_height, map);
  }

  sol::function Engine::check_if_lua_function_defined(sol::this_state s, const std::string &name)
  {
    sol::state_view lua(s);
    sol::function lua_func = lua[name];
    if (!(lua_func.valid() && lua_func.get_type() == sol::type::function))
    {
      fmt::println("game script does not define the '{}' method.", name);
      return nullptr;
    }

    return lua_func;
  }

  void Engine::setup_lua_api(sol::this_state s)
  {
    sol::state_view lua(s);

    lua.set_function("get_sprite_info", [&](std::string sprite_sheet_name, sol::this_state s)
                     { if(sprite_sheets->contains(sprite_sheet_name)) {
                          (*sprite_sheets)[sprite_sheet_name]->get_sprites_as_lua_table(s); } });
    lua.set_function("draw_text", [&](const std::string &t, int x, int y)
                     { draw_text(t, x, y); });
    lua.set_function("draw_text_with_color", [&](const std::string &t, int x, int y, int r, int g, int b, int a)
                     { draw_text(t, x, y, r, g, b, a); });
    lua.set_function("draw_sprite", [&](const std::string &spritesheet_name, int sprite_id, int x, int y)
                     { draw_sprite(spritesheet_name, sprite_id, x, y, 0); });
    lua.set_function("draw_sprite_scaled", [&](const std::string &spritesheet_name, int sprite_id, int x, int y, int scale_factor)
                     { draw_sprite(spritesheet_name, sprite_id, x, y, scale_factor); });
    lua.set_function("draw_sprite_sheet", [&](const std::string &spritesheet_name, int x, int y)
                     { auto ss_i = sprite_sheets->find(spritesheet_name);
                       if (ss_i != sprite_sheets->end())
                       { ss_i->second->draw_sprite_sheet(renderer, x, y); } });
    lua.set_function("set_draw_color", [&](int r, int g, int b, int a)
                     { set_draw_color(renderer, r, g, b, a); });
    lua.set_function("draw_point", [&](int x, int y)
                     { draw_point(renderer, x, y); });
    lua.set_function("draw_rect", [&](int x, int y, int w, int h)
                     { draw_rect(renderer, x, y, w, h); });
    lua.set_function("draw_filled_rect", [&](int x, int y, int w, int h)
                     { draw_filled_rect(renderer, x, y, w, h); });
    lua.set_function("draw_filled_rect_with_color", [&](int x, int y, int w, int h, int r, int g, int b, int a)
                     { draw_filled_rect_with_color(renderer, x, y, w, h, r, g, b, a); });
    lua.set_function("draw_graphic", [&](const std::string &path, int window_width, int x, int y, bool centered, int scale_factor)
                     { draw_graphic(renderer, path, window_width, x, y, centered, scale_factor); });
    lua.set_function("play_sound", [&](const std::string &name)
                     { play_sound(name); });
    lua.set_function("get_random_number", [&](int min, int max)
                     { return generate_random_int(min, max); });
    lua.set_function("generate_uuid", [&]()
                     { return generate_uuid(); });
    lua.set_function("generate_map", [&](const std::string &name, int map_width, int map_height)
                     { auto map = generate_map(name, map_width, map_height);
                       current_map_info.name = name;
                       current_map_info.map = map;
                       maps->push_back(map); });
    lua.set_function("get_random_point_on_map", [&](sol::this_state s)
                     { sol::state_view lua(s);
                       if (current_map_info.map != nullptr)
                       {
                         roguely::common::Point point{0,0};

                         do
                         {
                            point = current_map_info.map->get_random_point({0});
                         } while (!entity_manager->lua_is_point_unique(point));

                         return lua.create_table_with("x", point.x, "y", point.y);
                       }
                       return lua.create_table(); });
    lua.set_function("set_map", [&](const std::string &name)
                     { auto map = find_map(name);
                       if (map != nullptr)
                       {
                         current_map_info.map = map;
                         current_map_info.name = name;
                       } });
    lua.set_function("draw_visible_map", [&](const std::string &name, const std::string &ss_name, sol::function draw_map_callback)
                     { if (current_map_info.name != name)
                       {
                         auto map = find_map(name);

                         if (map != nullptr)
                         {
                           current_map_info.map = map;
                           current_map_info.name = name;
                         }
                       }

                       if (current_map_info.name == name)
                       {
                         current_map_info.map->draw_map(renderer, current_dimension, sprite_sheets->at(ss_name),
                                                        [&](int rows, int cols, int dx, int dy, int cell_id, int light_cell, int scale_factor)
                                                        {
                                                          auto draw_map_callback_result = draw_map_callback(rows, cols, dx, dy, cell_id, light_cell, scale_factor);
                                                          if (!draw_map_callback_result.valid())
                                                          {
                                                            sol::error err = draw_map_callback_result;
                                                            fmt::println("Lua script error: {}", err.what());
                                                          }
                                                        });
                       } });
    lua.set_function("draw_full_map", [&](const std::string &name, int x, int y, int a, sol::function draw_map_callback)
                     { if (current_map_info.name != name)
                       {
                         auto map = find_map(name);

                         if (map != nullptr)
                         {
                           current_map_info.map = map;
                           current_map_info.name = name;
                         }
                       }

                       if (current_map_info.name == name)
                       {
                         current_map_info.map->draw_map(renderer, current_dimension, x, y, a,
                                                        [&](int rows, int cols, int cell_id)
                                                        {
                                                          auto draw_map_callback_result = draw_map_callback(rows, cols, cell_id);
                                                          if (!draw_map_callback_result.valid())
                                                          {
                                                            sol::error err = draw_map_callback_result;
                                                            fmt::println("Lua script error: {}", err.what());
                                                          }
                                                        });
                       } });
    lua.set_function("add_entity", [&](const std::string &group_name, const std::string &name, sol::table components, sol::this_state s)
                     { auto entity = std::make_shared<roguely::ecs::Entity>(name);
                       auto components_copy = entity_manager->copy_table(components, s);
                       auto lua_component = std::make_shared<roguely::components::LuaComponent>("lua component", components_copy, s);
                       entity->add_component(lua_component);
                       entity_manager->add_entity_to_group(group_name, entity, s); });
    lua.set_function("remove_entity", [&](const std::string &entity_group_name, const std::string &entity_id)
                     { entity_manager->remove_entity(entity_group_name, entity_id); });
    lua.set_function("remove_component", [&](const std::string &entity_group_name, const std::string &entity_name, const std::string &component_name)
                     { entity_manager->remove_lua_component(entity_group_name, entity_name, component_name); });
    lua.set_function("get_component_value", [&](const std::string &entity_group_name, const std::string &entity_name, const std::string &component_name, const std::string &key, sol::this_state s)
                     {
                       sol::state_view lua(s);
                       auto entity = entity_manager->get_entity_by_name(entity_group_name, entity_name);
                       if (entity != nullptr)
                       {
                          auto component = entity->find_first_component_by_type<roguely::components::LuaComponent>();
                          if (component != nullptr)
                          {
                            auto lua_component = component->get_property<sol::table>(component_name);

                            if (lua_component != sol::nil)
                            {
                              return (sol::object)lua_component[key];
                            }
                          }
                       }
                       return (sol::object)lua.create_table(); });
    lua.set_function("set_component_value", [&](const std::string &entity_group_name, const std::string &entity_name, const std::string &component_name, const std::string &key, sol::object value, sol::this_state s)
                     {
                       auto entity = entity_manager->get_entity_by_name(entity_group_name, entity_name);
                       if (entity != nullptr)
                       {
                         auto component = entity->find_first_component_by_type<roguely::components::LuaComponent>();
                         if (component != nullptr)
                         {
                           auto lua_component = component->get_property<sol::table>(component_name);
                           if (lua_component != sol::nil)
                           {
                             lua_component.set(key, value);
                           }
                         }
                       } });
    lua.set_function("update_player_viewport", [&](int x, int y, int width, int height)
                     { current_dimension = update_player_viewport(
                           {x, y},
                           {current_map_info.map->get_width(), current_map_info.map->get_height()},
                           {width, height}); });
    lua.set_function("get_text_extents", [&](const std::string &t, sol::this_state s)
                     { sol::state_view lua(s);
                       sol::table extents_table = lua.create_table();
                       if(!default_font.expired()) {
                        auto extents = default_font.lock()->get_text_extents(t);
                        extents_table.set("width", extents.width);
                        extents_table.set("height", extents.height);
                       }
                       return extents_table; });
    lua.set_function("add_system", [&](const std::string &name, sol::function system_callback)
                     { systems->insert({name, system_callback}); });
    lua.set_function("get_random_key_from_table", [&](sol::table table)
                     {
                       if(table.valid())
                       {
                          std::vector<std::string> keys = {};
                          table.for_each([&](sol::object key, sol::object value)
                                         { keys.push_back(key.as<std::string>()); });
                          std::random_device rd;
                          std::mt19937 gen(rd());
                          std::uniform_int_distribution<> dis(0, static_cast<int>(keys.size()) - 1);
                          return keys[dis(gen)];
                       }
                       return std::string{}; });
    lua.set_function("find_entity_with_name", [&](const std::string &group_name, const std::string &name)
                     { return entity_manager->get_lua_entity(group_name, name); });
    lua.set_function("get_overlapping_points", [&](const std::string &entity_name, int x, int y, sol::function point_callback)
                     { return entity_manager->lua_for_each_overlapping_point(entity_name, x, y, point_callback); });
    lua.set_function("get_blocked_points", [&](const std::string &entity_group, int x, int y, const std::string &direction, sol::this_state s)
                     { auto blocked_points = entity_manager->get_lua_blocked_points(entity_group, x, y, direction, s);
                       if(!blocked_points.empty() && current_map_info.map != nullptr)
                       {
                         current_map_info.map->trigger_redraw();
                       }
                       return blocked_points; });
    lua.set_function("is_within_viewport", [&](int x, int y)
                     { return is_within_viewport(x, y); });
    lua.set_function("force_redraw_map", [&]()
                     { if(current_map_info.map != nullptr)
                       { current_map_info.map->trigger_redraw(); } });
    lua.set_function("add_font", [&](const std::string &name, const std::string &font_path, int font_size)
                     {
                       auto text = std::make_shared<roguely::common::Text>();
                       text->load_font(font_path, font_size);
                       texts->insert({name, std::move(text)});
                       default_font = text; });
    lua.set_function("set_font", [&](const std::string &name)
                     { auto text = texts->find(name);
                       if(text != texts->end())
                       { default_font = std::move(text->second); } });
    lua.set_function("get_adjacent_points", [&](int x, int y, sol::this_state s)
                     { sol::state_view lua(s);
                       std::vector<roguely::common::Point> points = {
                           /* UP    */ {x, y - 1},
                           /* DOWN  */ {x, y + 1},
                           /* LEFT  */ {x - 1, y},
                           /* RIGHT */ {x + 1, y}};

                       auto is_up_blocked =
                        entity_manager->lua_is_point_unique(points[0]) && current_map_info.map->is_point_blocked(points[0].x, points[0].y);
                       auto is_down_blocked =
                           entity_manager->lua_is_point_unique(points[1]) && current_map_info.map->is_point_blocked(points[1].x, points[1].y);
                       auto is_left_blocked =
                           entity_manager->lua_is_point_unique(points[2]) && current_map_info.map->is_point_blocked(points[2].x, points[2].y);
                       auto is_right_blocked =
                           entity_manager->lua_is_point_unique(points[3]) && current_map_info.map->is_point_blocked(points[3].x, points[3].y);
                           entity_manager->lua_is_point_unique(points[3]) && current_map_info.map->is_point_blocked(points[3].x, points[3].y);

                       sol::table adjacent_points = lua.create_table_with(
                           "up", lua.create_table_with("blocked", is_up_blocked, "x", points[0].x, "y", points[0].y),
                           "down", lua.create_table_with("blocked", is_down_blocked, "x", points[1].x, "y", points[1].y),
                           "left", lua.create_table_with("blocked", is_left_blocked, "x", points[2].x, "y", points[2].y),
                           "right", lua.create_table_with("blocked", is_right_blocked, "x", points[3].x, "y", points[3].y));
                       return adjacent_points; });
    lua.set_function("map_to_world", [&](int x, int y, const std::string &ss_name, sol::this_state s)
                     { sol::state_view lua(s);
                       sol::table table = lua.create_table();
                       if(current_map_info.map != nullptr)
                       {
                          auto point = current_map_info.map->map_to_world(x, y, current_dimension, sprite_sheets->at(ss_name));
                          table.set("x", point.x);
                          table.set("y", point.y);
                       }
                       return table; });
    lua.set_function("set_highlight_color", [&](const std::string &ss_name, int r, int g, int b)
                     { if (sprite_sheets->contains(ss_name))
                       { (*sprite_sheets)[ss_name]->set_highlight_color(r, g, b); } });
    lua.set_function("reset_highlight_color", [&](const std::string &ss_name)
                     { if (sprite_sheets->contains(ss_name))
                       { (*sprite_sheets)[ss_name]->reset_highlight_color();
                          if (current_map_info.map != nullptr)
                          {
                            current_map_info.map->trigger_redraw();
                          } } });
  }
}
