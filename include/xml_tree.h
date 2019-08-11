#ifndef AI_YANG_XML_TREE_H
#define AI_YANG_XML_TREE_H

static const char* xml_text = R"(

<root main_tree_to_execute="BehaviorTree">
    <!-- ////////// -->
    <BehaviorTree ID="BehaviorTree">
        <ReactiveSequence name="Root">
            <ReactiveSequence name="Strategy">
                <Action ID="PredictEnemyNowLoc"/>
                <Action ID="CalculateShortestPath"/>
                <Action ID="AssignTask"/>
            </ReactiveSequence>
            <ReactiveSequence name="Actions">
                <Action ID="EatEnemy"/>
                <Action ID="ExploreMap"/>
                <Action ID="EatPower"/>
                <Action ID="RunAway"/>
            </ReactiveSequence>
            <Action ID="MakeDecision"/>
        </ReactiveSequence>
    </BehaviorTree>
    <!-- ////////// -->
    <TreeNodesModel>
        <Action ID="AssignTask"/>
        <Action ID="CalculateShortestPath"/>
        <Action ID="EatEnemy"/>
        <Action ID="EatPower"/>
        <Action ID="ExploreMap"/>
        <Action ID="MakeDecision"/>
        <Action ID="PredictEnemyNowLoc"/>
        <Action ID="RunAway"/>
    </TreeNodesModel>
    <!-- ////////// -->
</root>


 )";

#endif //AI_YANG_XML_TREE_H
