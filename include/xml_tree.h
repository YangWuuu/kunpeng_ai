#ifndef AI_YANG_XML_TREE_H
#define AI_YANG_XML_TREE_H

static const char* xml_text = R"(

<root main_tree_to_execute="BehaviorTree">
    <!-- ////////// -->
    <BehaviorTree ID="BehaviorTree">
        <ReactiveFallback>
            <ReactiveSequence name="myMode">
                <Condition ID="IsMyMode"/>
                <SubTree ID="PowerOrRandom"/>
            </ReactiveSequence>
            <ReactiveSequence name="enemyMode">
                <Inverter>
                    <Condition ID="IsMyMode"/>
                </Inverter>
                <ReactiveFallback>
                    <ReactiveSequence name="enemyAround">
                        <Condition ID="IsEnemyAround"/>
                        <Action ID="RunAway"/>
                    </ReactiveSequence>
                    <ReactiveSequence name="notEnemyAround">
                        <Inverter>
                            <Condition ID="IsEnemyAround"/>
                        </Inverter>
                        <SubTree ID="PowerOrRandom"/>
                    </ReactiveSequence>
                </ReactiveFallback>
            </ReactiveSequence>
        </ReactiveFallback>
    </BehaviorTree>
    <!-- ////////// -->
    <BehaviorTree ID="PowerOrRandom">
        <ReactiveFallback>
            <ReactiveSequence name="powerAround">
                <Condition ID="IsPowerAround"/>
                <Action ID="MoveToNearestPower"/>
            </ReactiveSequence>
            <ReactiveSequence>
                <Inverter>
                    <Condition ID="IsPowerAround"/>
                </Inverter>
                <Action ID="RandomWalk"/>
            </ReactiveSequence>
        </ReactiveFallback>
    </BehaviorTree>
    <!-- ////////// -->
    <TreeNodesModel>
        <Condition ID="IsEnemyAround"/>
        <Condition ID="IsMyMode"/>
        <Condition ID="IsPowerAround"/>
        <Action ID="MoveToNearestPower"/>
        <SubTree ID="PowerOrRandom"/>
        <Action ID="RandomWalk"/>
        <Action ID="RunAway"/>
    </TreeNodesModel>
    <!-- ////////// -->
</root>


 )";

#endif //AI_YANG_XML_TREE_H
