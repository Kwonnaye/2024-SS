## 진행 상황 로깅 추가
## 모든 @ 다 삭제 ! (예외없음)
import os
import re


def clean_and_split_text(input_dir, output_dir, chunk_size=5000):
    # 입력 디렉토리에서 _cleaned2.txt가 붙은 파일만 찾기
    files = [f for f in os.listdir(input_dir) if f.endswith('_cleaned2.txt')]

    # 각 파일 처리
    for file_index, file_name in enumerate(files):
        file_path = os.path.join(input_dir, file_name)
        print(f"Processing {file_index + 1}/{len(files)}: {file_name}")

        with open(file_path, 'r', encoding='utf-8') as file:
            text = file.read()

        # '@'로 시작하는 모든 단어 및 '/' 문자 제거
        text = re.sub(r'@\w+', '', text)  # @로 시작하는 단어 제거
        text = text.replace('/', '')  # '/' 문자 제거

        chunks = []
        while len(text) > 0:
            take = text[:chunk_size]
            if len(text) > chunk_size:
                last_period = take.rfind('.')
                last_question = take.rfind('?')
                last_exclamation = take.rfind('!')
                safe_cut = max(last_period, last_question, last_exclamation) + 1

                chunks.append(text[:safe_cut].strip())
                text = text[safe_cut:].strip()
            else:
                chunks.append(text.strip())
                break

        # 새 파일로 결과 저장
        for i, chunk in enumerate(chunks):
            output_file_name = f"{os.path.splitext(file_name)[0]}_part_{i + 1}.txt"
            output_path = os.path.join(output_dir, output_file_name)
            with open(output_path, 'w', encoding='utf-8') as output_file:
                output_file.write(chunk)

        print(f"Finished processing {file_name}")


# 스크립트 실행 예
input_directory = r'C:\Users\PC00\Desktop\AICOSS_doodle\TS1사회'
output_directory = r'C:/Users/PC00/Desktop/doodle_clone/cleaned-data/TS1사회2'
os.makedirs(output_directory, exist_ok=True)  # 출력 디렉토리가 없으면 생성
clean_and_split_text(input_directory, output_directory)
