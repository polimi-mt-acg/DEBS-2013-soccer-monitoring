# -*- coding: utf-8 -*-
import os
import csv
import fileinput
from pprint import pprint

# http://debs.org/debs-2013-grand-challenge-soccer-monitoring/
# Answer 9:
# 1st Half:
# Start of Game: 10753295594424116
# End of Game: 12557295594424116
# Without Ball: 12398************

# 2nd Half:
# Start of Game: 13086639146403495
# End of Game: 14879639146403495

game_timestamps = {
    'half_1': {
        'start': 10753295594424116,
        'end': 12557295594424116,
    },
    'half_2': {
        'start': 13086639146403495,
        'end': 14879639146403495,
    }
}


def load_half_game_interrupts(filename='1st Half.csv', valid_ids=['2011', '2010'], half_index='half_1'):
    '''
    Load the game interrupt events for one of the halves of the match.
    Return: list of lists. Each list is one record with timestamp converted.
    '''
    game_interrupts = []

    with open('./../original/referee-events/Game Interruption/' + filename, newline='') as csv_file:
        lines = csv.reader(csv_file, delimiter=';')
        for line in lines:
            # filter lines, select only those starting with the id
            if line and line[0] in valid_ids:
                time = line[2]
                if time == '0':
                    hh = mm = ss = ms = 0
                elif len(time) == 12:
                    hh = int(time[0:2])
                    mm = int(time[3:5])
                    ss = int(time[6:8])
                    ms = int(time[9:12])

                delta_timestamp = 0
                delta_timestamp += hh * 60 * int(1e7)
                delta_timestamp += mm * 60 * int(1e7)
                delta_timestamp += ss * int(1e7)
                delta_timestamp += ms * int(1e4)

                timestamp = game_timestamps[half_index]['start'] + \
                    delta_timestamp

                game_interrupts.append([
                    'GI',  # game interrupt event type
                    line[0],  # event_id;
                    line[1],  # event_name;
                    line[2],  # event_time_real;
                    str(timestamp),  # event timestamp;
                    line[3],  # event_counter;
                    line[4],  # comment
                ])

    return game_interrupts


def load_game_interrupts():
    '''
    Load the game interrupt events for one of the halves of the match.
    Return: list of lists. Chronologically ordered lists of game interrupt events with timestamp converted
    '''

    # load 1st half game interrupts
    gi_1 = load_half_game_interrupts(
        filename='1st Half.csv',
        valid_ids=['2011', '2010'],
        half_index='half_1'
    )

    # load 2nd half game interrupts
    gi_2 = load_half_game_interrupts(
        filename='2nd Half.csv',
        valid_ids=['6014', '6015'],
        half_index='half_2'
    )

    two_half_merged = gi_1 + gi_2

    # sort events
    game_interrupts = sorted(two_half_merged, key=lambda x: x[4])
    # pprint(game_interrupts)
    return game_interrupts


def process_full_game(source_filepath='', target_filename='full-game'):
    '''
    Perform preprocessing on the full-game file
    '''

    print('=============================================')
    print(f'  {source_filepath} FILE PROCESSING')
    print('=============================================')
    print('Starting preprocessing...')
    print('#lines\tFile')
    os.system(f'wc -l {source_filepath}')

    gi = load_game_interrupts()
    # [
    #     [
    #         'GI',
    #         '2010',
    #         'Game Interruption Begin',
    #         '0',
    #         10753295594424116,
    #         '1',
    #         'empty'
    #     ],
    #     [
    #         'GI',
    #         '2011',
    #         'Game Interruption End',
    #         '00:00:03.092',
    #         10753295625344116,
    #         '1',
    #         'empty'
    #     ],
    #     ...
    # ]

    # let's reverse the list in order to pop element and get the first
    gi.reverse()

    # take first element
    gi_to_insert = gi.pop()

    # open target file
    with open(target_filename, 'w') as out:
        # open input file
        with fileinput.input(files=(source_filepath)) as csv_file:
            lines = csv.reader(csv_file)
            for line in lines:
                line_no = fileinput.lineno()

                # flush buffer and save to disk every n rows
                if line_no % 100000 == 0:
                    ll_no = "{:,}".format(line_no)
                    print(f'processing reached line: {ll_no}', flush=True)
                    out.flush()

                # add SE event type and create a string from the final list
                n_line = ','.join(['SE'] + line)

                # if
                if gi_to_insert and int(line[1]) >= int(gi_to_insert[4]):
                    print(
                        f'line {line_no}: Inserted Game Interruption event: {gi_to_insert}', flush=True)

                    gi_line = ','.join(gi_to_insert)
                    n_line = gi_line + '\n' + n_line

                    if len(gi):
                        gi_to_insert = gi.pop()
                    else:
                        gi_to_insert = None

                out.write(n_line + '\n')

    print('\n================================================================')
    print(f'  {source_filepath} FILE PROCESSING COMPLETED')
    print('================================================================\n\n')

    if len(gi):
        print('Not all the Game Interrupt events have been inserted.\n\nNot inserted are:')
        pprint(gi)
    else:
        print('All the Game Interrupt events have been inserted.')


if __name__ == '__main__':
    process_full_game(source_filepath='./../original/full-game')
