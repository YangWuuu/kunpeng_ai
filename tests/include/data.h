#include <string>
#include <vector>

namespace data{
    std::string leg_start = R"(
    {"msg_data":{"map":{"height":20,"meteor":[{"x":18,"y":1},{"x":19,"y":1},{"x":7,"y":4},{"x":8,"y":4},{"x":11,"y":4},{"x":12,"y":4},{"x":4,"y":7},{"x":15,"y":7},{"x":4,"y":8},{"x":15,"y":8},{"x":4,"y":11},{"x":15,"y":11},{"x":4,"y":12},{"x":15,"y":12},{"x":7,"y":15},{"x":8,"y":15},{"x":11,"y":15},{"x":12,"y":15},{"x":0,"y":18},{"x":1,"y":18}],"tunnel":[{"direction":"up","x":5,"y":5},{"direction":"right","x":6,"y":5},{"direction":"right","x":7,"y":5},{"direction":"right","x":8,"y":5},{"direction":"right","x":9,"y":5},{"direction":"right","x":10,"y":5},{"direction":"right","x":11,"y":5},{"direction":"right","x":12,"y":5},{"direction":"right","x":13,"y":5},{"direction":"right","x":14,"y":5},{"direction":"up","x":5,"y":6},{"direction":"down","x":14,"y":6},{"direction":"up","x":5,"y":7},{"direction":"down","x":14,"y":7},{"direction":"up","x":5,"y":8},{"direction":"down","x":14,"y":8},{"direction":"up","x":5,"y":9},{"direction":"down","x":14,"y":9},{"direction":"up","x":5,"y":10},{"direction":"down","x":14,"y":10},{"direction":"up","x":5,"y":11},{"direction":"down","x":14,"y":11},{"direction":"up","x":5,"y":12},{"direction":"down","x":14,"y":12},{"direction":"up","x":5,"y":13},{"direction":"down","x":14,"y":13},{"direction":"left","x":5,"y":14},{"direction":"left","x":6,"y":14},{"direction":"left","x":7,"y":14},{"direction":"left","x":8,"y":14},{"direction":"left","x":9,"y":14},{"direction":"left","x":10,"y":14},{"direction":"left","x":11,"y":14},{"direction":"left","x":12,"y":14},{"direction":"left","x":13,"y":14},{"direction":"down","x":14,"y":14}],"vision":3,"width":20,"wormhole":[{"name":"A","x":19,"y":0},{"name":"b","x":13,"y":6},{"name":"a","x":6,"y":13},{"name":"B","x":0,"y":19}]},"teams":[{"force":"beat","id":1111,"players":[0,1,2,3]},{"force":"think","id":1112,"players":[4,5,6,7]}]},"msg_name":"leg_start"}
    )";
    std::vector<std::string> msg_round = {
    R"(
    {"msg_data":{"mode":"beat","players":[{"id":4,"score":0,"sleep":0,"team":1112,"x":1,"y":0},{"id":5,"score":0,"sleep":0,"team":1112,"x":10,"y":0},{"id":6,"score":0,"sleep":0,"team":1112,"x":9,"y":19},{"id":7,"score":0,"sleep":0,"team":1112,"x":18,"y":19}],"power":[{"point":1,"x":3,"y":2},{"point":1,"x":7,"y":2},{"point":1,"x":12,"y":2},{"point":1,"x":2,"y":3},{"point":1,"x":17,"y":16},{"point":1,"x":7,"y":17},{"point":1,"x":12,"y":17},{"point":1,"x":16,"y":17}],"round_id":0,"teams":[{"id":1111,"point":0,"remain_life":4},{"id":1112,"point":0,"remain_life":4}]},"msg_name":"round"}
    )", 
    R"(
    {"msg_data":{"mode":"beat","players":[{"id":4,"score":0,"sleep":0,"team":1112,"x":2,"y":0},{"id":5,"score":0,"sleep":0,"team":1112,"x":11,"y":0},{"id":6,"score":0,"sleep":0,"team":1112,"x":8,"y":19},{"id":7,"score":0,"sleep":0,"team":1112,"x":18,"y":19}],"power":[{"point":1,"x":3,"y":2},{"point":1,"x":12,"y":2},{"point":1,"x":2,"y":3},{"point":1,"x":17,"y":16},{"point":1,"x":7,"y":17},{"point":1,"x":16,"y":17}],"round_id":1,"teams":[{"id":1111,"point":0,"remain_life":4},{"id":1112,"point":0,"remain_life":4}]},"msg_name":"round"}    
    )"
    };


}
