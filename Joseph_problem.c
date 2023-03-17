#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(void)
{
    int people_number = 0;
    int i = 0;
    int people_start = 0;
    int run_direction = 0;
    int skip_number = 0;

    struct ring_s{
        struct ring_s *next;
        struct ring_s *front;
        int people;
    };

    struct ring_s *people_ring = NULL;
    struct ring_s *tmp = NULL;


    printf("input number of people: ");
    scanf("%d", &people_number);

    for (i = 0; i < people_number; i++)
    {
        tmp = (struct ring_s *)malloc(sizeof(struct ring_s));
        tmp->people = i+1;
        tmp->front = tmp;
        tmp->next = tmp;

        if (people_ring == NULL)
        {
            people_ring = tmp;
        }
        else
        {
            tmp->next = people_ring->next;
            people_ring->next->front = tmp;
            people_ring->next = tmp;
            tmp->front = people_ring;
            people_ring = people_ring->next;
        }        
    }

    // printf("people order is: ");
    for (i = 0; i < people_number; i++)
    {
        people_ring = people_ring->next;
        printf("%d ", people_ring->people);
    }

    // printf(" now at:%d\r\n", people_ring->people);
    for (i = 0; i < people_number; i++)
    {
        printf("%d ", people_ring->people);
        people_ring = people_ring->front;
    }
    people_ring = people_ring->next;
    
    // printf("now at %d\r\n", people_ring->people);

    printf("input start at, dirction, skip number: ");
    scanf("%d %d %d", &people_start, &run_direction, &skip_number);

    // printf("your input is: %d, %d, %d\r\n", people_start, run_direction, skip_number);

    for (i = 1; i < people_start; i++)
    {
        people_ring = people_ring->next;
    } 
    // printf("now at: %d kill order: ", people_ring->people);

    while (people_number > 1)
    {
        if (run_direction == 0)
        {
            for (i = 0; i < skip_number; i++)
            {
                people_ring = people_ring->next;
            }
            tmp = people_ring->next;
            printf("%d ", tmp->people);
            people_ring->next = people_ring->next->next;
            people_ring = people_ring->next;
        }
        else
        {
            for (i = skip_number; i > 0; i--)
            {
                people_ring = people_ring->front;
            }
            tmp = people_ring->front;
            printf("%d ", tmp->people);
            people_ring->front = people_ring->front->front;
            people_ring = people_ring->front;
        }
        people_number--;
        free(tmp);
    }
    printf("--- (%d live)\r\n", people_ring->people);

    free(people_ring);

    return 0;
}