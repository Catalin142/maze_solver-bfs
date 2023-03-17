#include "framework.h"

#include <fstream>
#include <string>
#include <queue>

using vertex = fm::v2<int>;

struct node
{
	vertex v;
	uint8_t type;
	bool visited;
	uint32_t length;
};

class maze_solver : public fm::application
{
public:
	maze_solver() = default;
	~maze_solver()
	{
		for (int i = 0; i < height; i++)
			free(maze[i]);
		free(maze);
	}

	void on_create() override
	{
		// calculate labirinth widht / height
		std::ifstream in_file("labirint.txt");
		if (!in_file.is_open())
			return;

		std::string line;
		std::getline(in_file, line);
		width = line.size();

		in_file.clear();
		in_file.seekg(0);

		while (std::getline(in_file, line))
			height++;

		maze = (node**)malloc(height * sizeof(node*));
		if (!maze)
			return;

		for (int i = 0; i < height; i++)
		{
			maze[i] = (node*)malloc(width * sizeof(node));
			if (!maze[i])
				return;
		}

		in_file.clear();
		in_file.seekg(0);

		int cur_y = 0;
		while (std::getline(in_file, line))
		{
			for (int i = 0; i < width; i++)
			{
				if (line[i] == '1')
					maze[cur_y][i].type = 1;
				else if (line[i] == ' ')
					maze[cur_y][i].type = 0;
				if (line[i] == 'S')
				{
					maze[cur_y][i].type = 0;
					start_point = &maze[cur_y][i];
				}
				if (line[i] == 'F')
				{
					maze[cur_y][i].type = 0;
					end_point = &maze[cur_y][i];
				}

				maze[cur_y][i].v = vertex(i, cur_y);
				maze[cur_y][i].visited = false;
				maze[cur_y][i].length = 0;
			}
			cur_y++;
		}

		in_file.close();

		uint32_t len = 0;
		len = solve();
		if (len)
			path = trace(len);
		return;
	}

	void on_update(float dt) override
	{
		clear(fm::color(0xF9DBBB));

		start_render_x = screen_width() / 2 - (width / 2 * block_size);
		start_render_y = screen_height() / 2 - (height / 2 * block_size);

		for (int i = 0; i < height; i++)
			for (int j = 0; j < width; j++)
			{
				if (maze[i][j].type == 0 && maze[i][j].visited == false)
					draw_quad_fill(fm::color(0xF9DBBB), start_render_x + j * block_size, start_render_y + i * block_size, block_size, block_size);
				else if (maze[i][j].visited == true)
					draw_quad_fill(fm::color(0x2E3840), start_render_x + j * block_size, start_render_y + i * block_size, block_size, block_size);
				else if (maze[i][j].type == 1)
					draw_quad_fill(fm::color(0x4E6E81), start_render_x + j * block_size, start_render_y + i * block_size, block_size, block_size);
			}

		for (const node* n : path)
			draw_quad_fill(fm::color(0xFF0303), start_render_x + n->v.x * block_size, start_render_y + n->v.y * block_size, block_size, block_size);
	}

private:
	node** maze = nullptr;
	uint32_t width = 0, height = 0;
	std::vector<node*> path;

	node* start_point;
	node* end_point;

	const uint32_t block_size = 3;
	uint32_t start_render_x = 0;
	uint32_t start_render_y = 0;

private:
	uint32_t solve();
	std::vector<node*> trace(uint32_t count);

	bool reachable(const vertex& lhs, const vertex& rhs)
	{
		if (lhs.x + 1 == rhs.x || lhs.x - 1 == rhs.x ||
			lhs.y + 1 == rhs.y || lhs.y - 1 == rhs.y)
			return true;
	}

	void get_neighbours(const vertex& v, node** lst)
	{
		if (v.x != 0) // spatiu gol
			lst[0] = &maze[v.y][v.x - 1];
		if (v.x != width - 1)
			lst[1] = &maze[v.y][v.x + 1];
		if (v.y != 0)
			lst[2] = &maze[v.y - 1][v.x];
		if (v.y != height - 1)
			lst[3] = &maze[v.y + 1][v.x];
	}
};

int main()
{
	maze_solver app;

	if (app.initialize(L"maze solver", 1280, 720, 320, 200))
		app.start();

	return 0;
}

uint32_t maze_solver::solve()
{
	std::queue<node*> node_q;
	node_q.push(start_point);
	node* x = nullptr;
	while (!node_q.empty())
	{
		x = node_q.front();
		node_q.pop();

		if (x->visited) continue;

		x->visited = true;

		if (x->v.x == end_point->v.x && x->v.y == end_point->v.y) break;

		node* ngh[4]{ nullptr };
		get_neighbours(x->v, ngh);
		for (int i = 0; i < 4; i++)
		{
			if (ngh[i] != nullptr && !ngh[i]->visited && ngh[i]->type == 0)
				node_q.push(ngh[i]), ngh[i]->length = x->length + 1;
		}
	}

	if (end_point->visited == false)
		return 0;


	return x->length;
}

std::vector<node*> maze_solver::trace(uint32_t len)
{
	std::vector<node*> path;
	path.push_back(end_point);
	node* curr = end_point;

	while (curr->length != 0)
	{
		node* ngh[4]{ nullptr };
		get_neighbours(curr->v, ngh);
		for (int i = 0; i < 4; i++)
		{
			if (ngh[i] != nullptr && ngh[i]->type == 0 && ngh[i]->length == len - 1)
			{
				path.push_back(ngh[i]), curr = ngh[i];
				len--;
				break;
			}
		}
	}

	return path;
}
