import struct
import copy
import os

array_type = "const unsigned char"
array_length = "const unsigned int"

game_data = {
    'name': None,
    'data_ptr': None,
    'size': None
}

games_data = []

def main():
    with open('../src/roms.h', 'w') as out_file:
        out_file.write('#ifndef __ROMS_H__\n#define __ROMS_H__\n\n')
        for file in os.listdir('../games'):
            if file[-4:] == '.ch8':
                with open('../games/{}'.format(file), 'rb') as f:
                    data = f.read()
                    if(len(data) >= 4096 - 200):
                        continue
                    array_name = f'{file[:-4]}_rom'
                    out_file.write(f"{array_type} {array_name}[] = " "{\n")
                    for i, byte in enumerate(data):
                        out_file.write(f'0x{byte:02x}')
                        if i < len(data) - 1:
                            out_file.write(', ')
                        if i % 16 == 15:
                            out_file.write('\n\t')
                    out_file.write('\n};\n')
                    out_file.write(f"{array_length} {array_name}_len = {len(data)};\n\n")
                    current_game_data = game_data.copy()
                    current_game_data['name'] = file[:-4]
                    current_game_data['data_ptr'] = array_name
                    current_game_data['size'] = f"{len(data)}"
                    games_data.append(current_game_data)

        out_file.write("typedef struct {\n")
        out_file.write("\tconst char* name;\n")
        out_file.write("\tconst unsigned char* data;\n")
        out_file.write("\tunsigned int size;\n")
        out_file.write("} EmbeddedROM;\n\n")

        out_file.write('const EmbeddedROM g_roms[] = {\n')
        for index, game in enumerate(games_data):
            out_file.write(f'\t{{ "{game['name']}", {game['data_ptr']}, {game['size']} }}')
            if index != len(games_data) - 1:
                out_file.write(',')
            out_file.write('\n')
        out_file.write('};\n\n')
        out_file.write('#endif')

if __name__ == "__main__":
    main()
