
import os
import glob

# 삭제할 단어 목록
remove_words = ["/(bgm)", "/(idiom)", "@웃음","/(noise)"]

# 텍스트 파일이 있는 디렉토리
directory = "C:/Users/PC00/Desktop/AICOSS_doodle/TS2사회"

# 디렉토리 내의 모든 텍스트 파일을 처리
for file_name in glob.glob(os.path.join(directory, '*.txt')):
    with open(file_name, 'r', encoding='utf-8') as file:
        lines = file.readlines()

    # 각 라인에서 필요없는 단어를 제거
    modified_lines = []
    for line in lines:
        for word in remove_words:
            line = line.replace(word, "")
        modified_lines.append(line)

    # 결과를 새 파일에 저장
    with open(file_name.replace('.txt', '_cleaned.txt'), 'w', encoding='utf-8') as file:
        file.writelines(modified_lines)
print("필요없는 단어를 제거하였습니다")