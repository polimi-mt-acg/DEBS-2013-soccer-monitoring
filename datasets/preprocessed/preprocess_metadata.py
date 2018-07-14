# -*- coding: utf-8 -*-
import re
import os
import csv
import fileinput
from pprint import pprint


class _RegExLib:
    """Set up regular expressions"""
    # use https://regexper.com to visualise these if required
    _reg_section_balls = re.compile(r'.*Balls.*')
    _reg_section_team_A = re.compile(r'.*Team A.*')
    _reg_section_team_B = re.compile(r'.*Team B.*')

    _reg_balls_1st_half = re.compile(r'- 1st Half: ([0-9,\s]+).*')
    _reg_balls_2nd_half = re.compile(r'- 2nd Half: ([0-9,\s]+).*')
    # _reg_grade = re.compile(r'Grade = (.*)\n')
    # _reg_name_score = re.compile(r'(Name|Score)')

    _reg_player = re.compile(
        r'^(.*)\s\(Left Leg: ([0-9]+), Right Leg: ([0-9]+)\).*')
    _reg_goalkeeper = re.compile(
        r'^(.*)\s\(Left Leg: ([0-9]+), Right Leg: ([0-9]+), Left Arm: ([0-9]+), Right Arm: ([0-9]+)\).*')

    _reg_referee = re.compile(
        r'Referee \(Left Leg: ([0-9]+), Right Leg: ([0-9]+).*')

    __slots__ = [
        'section_balls',
        'section_team_A',
        'section_team_B',

        'balls_1st_half',
        'balls_2nd_half',

        'player',
        'goalkeeper',

        'referee',
    ]

    def __init__(self, line):
        # check whether line has a positive match with all of the regular expressions
        self.section_balls = self._reg_section_balls.match(line)
        self.section_team_A = self._reg_section_team_A.match(line)
        self.section_team_B = self._reg_section_team_B.search(line)

        self.balls_1st_half = self._reg_balls_1st_half.search(line)
        self.balls_2nd_half = self._reg_balls_2nd_half.search(line)

        self.player = self._reg_player.search(line)
        self.goalkeeper = self._reg_goalkeeper.search(line)

        self.referee = self._reg_referee.search(line)


def load_matedata_content(filepath):
    '''
    Parse metadata file and return a list of comma separated values.
    '''
    contents = []
    section = None

    with open(filepath, 'r') as file:
        for line in file.read().splitlines():
            reg_match = _RegExLib(line)

            if not section and reg_match.section_balls:
                section = 'BALLS'
                print('---------------------\nPARSED section: ', section)
                continue

            elif section == 'BALLS' and reg_match.section_team_A:
                section = 'TEAM_A'
                print('\n---------------------\nPARSED section: ', section)
                continue

            elif section == 'TEAM_A' and reg_match.section_team_B:
                section = 'TEAM_B'
                print('\n---------------------\nPARSED section: ', section)
                continue

            elif section == 'TEAM_B' and reg_match.referee:
                print('\n---------------------\nPARSED ', line)
                id1 = reg_match.referee.group(1)
                id2 = reg_match.referee.group(2)
                c = 'REFEREE,None,None,{},{},0,0'.format(id1, id2)
                contents.append(c)
                continue

            if section == 'BALLS':

                # balls 1st half
                if reg_match.balls_1st_half:
                    balls = reg_match.balls_1st_half.group(1).strip()
                    balls = balls.split(',')
                    for b in balls:
                        c = 'BALL,1,{}'.format(b.strip())
                        contents.append(c)
                    print('PARSED balls 1st half', balls)

                # balls 2nd half
                if reg_match.balls_2nd_half:
                    balls = reg_match.balls_2nd_half.group(1).strip()
                    balls = balls.split(',')
                    for b in balls:
                        c = 'BALL,2,{}'.format(b.strip())
                        contents.append(c)
                    print('PARSED balls 2nd half', balls)

            elif section == 'TEAM_A' or section == 'TEAM_B':
                team = 'A' if section == 'TEAM_A' else 'B'

                if reg_match.player:
                    name = reg_match.player.group(1).strip()
                    id1 = reg_match.player.group(2).strip()
                    id2 = reg_match.player.group(3).strip()
                    c = 'PLAYER,{},{},{},{},0,0'.format(team, name, id1, id2)
                    print('PARSED', c)
                    contents.append(c)

                if reg_match.goalkeeper:
                    name = reg_match.goalkeeper.group(1).strip()
                    id1 = reg_match.goalkeeper.group(2).strip()
                    id2 = reg_match.goalkeeper.group(3).strip()
                    id3 = reg_match.goalkeeper.group(4).strip()
                    id4 = reg_match.goalkeeper.group(5).strip()
                    c = 'PLAYER,{},{},{},{},{},{}'.format(
                        team, name, id1, id2, id3, id4)
                    print('PARSED', c)
                    contents.append(c)

    return contents


def process_metadata(source_filepath='', target_filepath='metadata'):
    '''
    Perform preprocessing of metadata file
    '''

    print('=============================================')
    print(f'  {source_filepath} FILE PROCESSING')
    print('=============================================')
    print('Starting preprocessing...')
    print('#lines\tFile')
    os.system(f'wc -l {source_filepath}')

    lines = load_matedata_content(filepath=source_filepath)

    # add line separator
    lines = map(lambda x: x + '\n', lines)

    # open target file
    with open(target_filepath, 'w') as out:
        out.writelines(lines)

    print('\n================================================================')
    print(f'  {source_filepath} FILE PROCESSING COMPLETED')
    print('================================================================\n\n')


if __name__ == '__main__':
    process_metadata(source_filepath='./../original/metadata.txt')
