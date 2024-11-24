#ifndef _LEVEL_H_
#define _LEVEL_H_
#include "types.h"
#include "ecs.h"

#include <vector>
#include <map>
#include <queue>
#include <set>

class LevelData
{
public:
    enum LevelTileType
    {
        WALL,
        DOOR,
        NUM_LEVEL_TILE_TYPES
    };
    struct LevelTile
    {
        LevelTileType type;
        int orientation;
    };
    int getTileIndex(Vec2i pos)
    {
        if ( data.find(pos) == data.end() )
        {
            return -1;
        }
        return data[pos].orientation;
    }

    LevelData() {}
    static LevelData load(std::string path, Vec2i offset)
    {
        LevelData level;
        int w = 0;
        int h = 0;
        try
        {
            std::ifstream istr(path, std::ios_base::in);
            int y = 0;
            for( std::string line; std::getline(istr, line); ++y)
            {
                printf("y: %2d ", y);
                for ( int x = 0; x < (int)line.length(); ++x )
                {
                    printf(">(%d,%d)<", x+offset.x,y+offset.y);
                    if ( x > w )
                    {
                        w = x;
                    }
                    switch (line[x])
                    {
                        case 'W' :
                            level.data[Vec2i{2*x+offset.x,2*y+offset.y}] = {WALL,0};
                            level.data[Vec2i{2*x+offset.x+1,2*y+offset.y}] = {WALL,0};
                            level.data[Vec2i{2*x+offset.x+1,2*y+offset.y+1}] = {WALL,0};
                            level.data[Vec2i{2*x+offset.x,2*y+offset.y+1}] = {WALL,0};
                            break;
                        case 'D' :
                            level.data[Vec2i{x+offset.x,y+offset.y}] = {DOOR,0};
                            break;
                        default:
                            break;
                    }
                }
                printf("\n");
                if ( y > h )
                {
                    h = y;
                }
            }
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }

        // index in tile-set to be used
        // indices correspond to the sum of neighbor positions
        //  1		1/2		2  
        //  1/2		x		2/8
        //   4		4/8     8
        for ( std::pair<const Vec2i, LevelTile> &tile : level.data )
        {
            int x = tile.first.x;
            int y = tile.first.y;
            int cnt = 0;
            cnt += (level.data.find(Vec2i{x, y - 1}) != level.data.end())
                    && (level.data.find(Vec2i{x - 1, y - 1}) != level.data.end())
                    && (level.data.find(Vec2i{x - 1, y}) != level.data.end())
                 ? 4 : 0;
            cnt += (level.data.find(Vec2i{x, y - 1}) != level.data.end())
                    && (level.data.find(Vec2i{x + 1, y - 1}) != level.data.end())
                    && (level.data.find(Vec2i{x + 1, y}) != level.data.end())
                 ? 8 : 0;
            cnt += (level.data.find(Vec2i{x, y + 1}) != level.data.end())
                    && (level.data.find(Vec2i{x - 1, y + 1}) != level.data.end())
                    && (level.data.find(Vec2i{x - 1, y}) != level.data.end())
                 ? 1 : 0;
            cnt += (level.data.find(Vec2i{x, y + 1}) != level.data.end())
                    && (level.data.find(Vec2i{x + 1, y + 1}) != level.data.end())
                    && (level.data.find(Vec2i{x + 1, y}) != level.data.end())
                 ? 2 : 0;
            tile.second.orientation = cnt;
            printf("LEVEL %2d %2d -> %d\n", x, y, cnt);
        }
        printf("LEVEL size: %ld\n", level.data.size());
        
        return level;
    }

    bool intersects(const Bounds &b)
    {
        return data.find(Vec2i{(int)b.pos.x, (int)b.pos.y}) != data.end()
                || data.find(Vec2i{(int)(b.pos.x + b.size.x), (int)b.pos.y}) != data.end()
                || data.find(Vec2i{(int)(b.pos.x + b.size.x), (int)(b.pos.y + b.size.y)}) != data.end()
                || data.find(Vec2i{(int)b.pos.x, (int)(b.pos.y + b.size.y)}) != data.end();
    }
    std::vector<std::pair<Vec2i, LevelTile>> getTilesInBounds(const Bounds &b)
    {
        std::vector<std::pair<Vec2i, LevelTile>> tiles;
        Vec2i min = {(int)b.pos.x, (int)b.pos.y};
        Vec2i max = {(int)std::ceil(b.pos.x + b.size.x), (int)(std::ceil(b.pos.y + b.size.y))};
        for ( int x = min.x; x < max.x ; ++x )
        {
            for ( int y = min.y; y < max.y; ++y )
            {
                Vec2i tmp{x,y};
                if ( data.find(tmp) != data.end() )
                {
                    tiles.push_back(std::pair<Vec2i, LevelTile>(tmp, data[tmp]));
                }
            }
        }
        printf("number of active tiles: %d\n", tiles.size());
        return tiles;
    }
    std::vector<Vec2> getPathTo(Vec2i start, Vec2i end)
    {
        struct WeightedVec2i{
            Vec2i pos;
            int distanceSquared;
            std::vector<Vec2i> path;
        };
        std::vector<Vec2i> path;
        std::set<Vec2i> seen;
        auto cmp = [&](WeightedVec2i l, WeightedVec2i r) { return l.distanceSquared > r.distanceSquared; };
        std::priority_queue<WeightedVec2i, std::vector<WeightedVec2i>, decltype(cmp)> pq(cmp);

        auto getWeightedVec2i = [] (Vec2i pos, Vec2i target, std::vector<Vec2i> path) -> WeightedVec2i
        {
            path.push_back(pos);
            return WeightedVec2i{pos, (target.x - pos.x) * (target.x - pos.x) + (target.y - pos.y) * (target.y - pos.y), path};
        };

        pq.push(getWeightedVec2i(start, end, {}));
        do
        {
            WeightedVec2i el = pq.top();
            pq.pop();
            printf("test path of length %lu to pos %d %d (target %d %d) pq size: %lu\n", el.path.size(), el.pos.x, el.pos.y, end.x, end.y, pq.size());
            if (end == el.pos)
            {
                el.path.push_back(el.pos);
                path = el.path;
                break;
            }
            for (int x = -1; x < 2; ++x)
            {
                for (int y = -1; y < 2; ++y)
                {
                    if ( not (x == 0 || y == 0) )
                        continue;
                    Vec2i next{el.pos.x + x, el.pos.y + y};
                    if (data.find(next) != data.end())
                    {
                        continue;
                    }
                    std::vector<Vec2i> tmp = el.path;
                    pq.push(getWeightedVec2i(next, end, tmp));
                }
            }
        } while (not pq.empty());
        std::vector<Vec2> vPath;
        for ( auto p : path )
        {
            vPath.push_back(Vec2{(float)p.x,(float)p.y});
        }
        return vPath;
    }
private:
    std::map<Vec2i, LevelTile> data;
};


#endif // _LEVEL_H_