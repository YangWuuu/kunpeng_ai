#include <gtest/gtest.h>
#include <numeric>
#include <vector>

#include <data.h>
#include <util.h>
#include <player.h>

using namespace std;

class PlayerTest : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        leg_start = data::leg_start;

    }
    string leg_start;
};

TEST_F(PlayerTest, Parse)
{
    Player player(8888, "my_ai");
    player.message_leg_start(cJSON_Parse(leg_start.c_str()));
}

int main(int argc, char *argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
