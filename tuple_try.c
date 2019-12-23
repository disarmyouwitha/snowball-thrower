#include <time.h>
#include <stdio.h>
#include <string.h>

typedef struct action_item { char _char; double _time; double _held; } action_item;

int main(void) 
{
    //-------
    int _action_length = 8;
    struct action_item action_list[8];
    struct action_item release_list[8];
    action_list[0]= (action_item){ 'W', 2.280, 0.988 };
    action_list[1]= (action_item){ 'A', 4.197, 0.144 };
    action_list[2]= (action_item){ 'S', 4.589, 0.072 };
    action_list[3]= (action_item){ 'D', 4.837, 0.096 };
    action_list[4]= (action_item){ 'W', 7.189, 0.112 };
    action_list[5]= (action_item){ 'D', 7.813, 0.112 };
    action_list[6]= (action_item){ 'S', 8.069, 0.088 };
    action_list[7]= (action_item){ 'A', 8.357, 0.112 };
    //---

    //-------

    time_t _start;
    time((time_t *)&_start);
    time_t time_taken;
    int _cnt = 0;

    while(1)
    {
        time_t _end;
        time((time_t *)&_end);

        time_taken = (_end - _start);
        action_item _action = action_list[_cnt];

        if(time_taken > _action._time)
        {
            printf("%c", _action._char);
            printf("\r\n");

            // [Last step]:
            //if(time_taken == action_list[_action_length-1]._time)
            if(_cnt==_action_length-1)
            {
                printf("[Fin.]");
                break;
            }
            _cnt++;
        }
    }
}