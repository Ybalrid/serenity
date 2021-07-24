/*
 * Copyright (c) 2020, the SerenityOS developers.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <cstdlib>
#include <cstring>
#include <ctime>

#include <LibGUI/Application.h>
#include <LibGUI/Menu.h>
#include <LibGUI/Painter.h>
#include <LibGUI/Widget.h>
#include <LibGUI/Window.h>

struct cell {
    enum class state {
        alive,
        dead
    };

    state current_state = state::dead;
};

static constexpr int cell_size = 8;
static constexpr int grid_w = 24;
static constexpr int grid_h = 16;

cell grid[grid_w][grid_h];
cell shadow_grid[grid_w][grid_h];

void seed_grid();
int count_alive_cells_around(int x, int y);
void generation();

class LifeWidget : public GUI::Widget {
    C_OBJECT(LifeWidget);

    int refresh_delay = 100;
    bool first = true;

public:
    virtual void paint_event(GUI::PaintEvent& event)
    {
        (void)event;
        GUI::Painter painter(*this);
        for (int x = 0; x < grid_w; ++x)
            for (int y = 0; y < grid_h; ++y)
                painter.fill_rect(Gfx::IntRect(x * cell_size,
                                      y * cell_size,
                                      cell_size,
                                      cell_size),
                    grid[x][y].current_state == cell::state::alive ? Color::Magenta : Color::Black);
        if (first) {
            start_timer(refresh_delay);
            first = false;
        }
    }

    virtual void timer_event(Core::TimerEvent&)
    {
        generation();
        update();
        stop_timer();
        start_timer(refresh_delay);
    }
};

void seed_grid()
{
    for (int x = 0; x < grid_w; ++x)
        for (int y = 0; y < grid_h; ++y)
            grid[x][y].current_state = rand() % 2 ? cell::state::alive : cell::state::dead;
}

int count_alive_cells_around(int x, int y)
{
    int cells = 0;

    for (int i = x - 1; i <= x + 1; ++i) {
        for (int j = y - 1; j <= y + 1; ++j) {
            if (i < 0 || i >= grid_w)
                continue;
            if (j < 0 || j >= grid_h)
                continue;
            if (i == x && j == y)
                continue;

            if (grid[i][j].current_state == cell::state::alive)
                cells++;
        }
    }

    return cells;
}

void generation()
{

    for (int x = 0; x < grid_w; ++x)
        for (int y = 0; y < grid_h; ++y) {
            int cells_around_me = count_alive_cells_around(x, y);

            if (cells_around_me < 2)
                shadow_grid[x][y].current_state = cell::state::dead;
            else if (cells_around_me == 3)
                shadow_grid[x][y].current_state = cell::state::alive;
            else if (cells_around_me > 3)
                shadow_grid[x][y].current_state = cell::state::dead;
        }

    memcpy(&grid[0][0], &shadow_grid[0][0], sizeof(cell) * grid_w * grid_h);
}

int main(int argc, char** argv)
{
    srand(time(nullptr));
    seed_grid();
    auto app = GUI::Application::construct(argc, argv);
    auto window = GUI::Window::construct();

    auto& life = window->set_main_widget<LifeWidget>();
    (void)life;

    window->set_title("Game of Life");
    window->resize(grid_w * cell_size, grid_h * cell_size);
    window->set_resizable(false);

    auto& menu = window->add_menu("File");

    menu.add_action(GUI::Action::create("Restart", [&](auto&) {
        seed_grid();
    }));

    menu.add_action(GUI::CommonActions::make_quit_action([&](auto&) {
        app->quit();
    }));

    window->show();

    return app->exec();
}
