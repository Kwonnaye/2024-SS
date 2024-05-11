import os
import re


def clean_and_save_new(input_dir):
    # 입력 디렉토리에서 _cleaned2.txt가 붙은 파일만 찾기
    files = [f for f in os.listdir(input_dir) if f.endswith('_cleaned2.txt')]

    # 각 파일 처리
    for file_name in files:
        file_path = os.path.join(input_dir, file_name)
        new_file_name = file_name.replace('_cleaned2.txt', '_cleaned3.txt')
        new_file_path = os.path.join(input_dir, new_file_name)

        with open(file_path, 'r', encoding='utf-8') as file:
            content = file.read()

        # '@'로 시작하는 모든 단어 및 '/' 문자 제거
        content = re.sub(r'@\w+', '', content)  # @로 시작하는 단어 제거
        updated_content = content.replace('/', '')  # '/' 문자 제거

        # 새 파일로 결과 저장
        with open(new_file_path, 'w', encoding='utf-8') as new_file:
            new_file.write(updated_content)

        print(f"Processed {file_name} and saved as {new_file_name}")


# 스크립트 실행 예
input_directory = r'C:/Users/PC00/Desktop/AICOSS_doodle/TS1문화'
clean_and_save_new(input_directory)
