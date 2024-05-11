import os
import re


def clean_and_split_text(input_dir, output_dir, chunk_size=5000):
    # 입력 디렉토리에서 특정 패턴을 포함하는 파일 목록 가져오기
    files = [f for f in os.listdir(input_dir) if os.path.isfile(os.path.join(input_dir, f)) and '_cleaned2.txt' in f]

    # 모든 파일 처리
    for file_name in files:
        file_path = os.path.join(input_dir, file_name)
        with open(file_path, 'r', encoding='utf-8') as file:
            text = file.read()

        # '@이름n' 패턴과 '/' 문자 제거
        text = re.sub(r'@이름\d+', '', text)
        text = text.replace('/', '')

        # 문장 끝을 고려하여 5000자씩 분할
        chunks = []
        while len(text) > 0:
            take = text[:chunk_size]
            if len(text) > chunk_size:
                # 마지막 문장 끝을 찾기
                last_period = take.rfind('.')
                last_question = take.rfind('?')
                last_exclamation = take.rfind('!')
                safe_cut = max(last_period, last_question, last_exclamation) + 1
                chunks.append(text[:safe_cut].strip())
                text = text[safe_cut:].strip()
            else:
                chunks.append(text.strip())
                break

        # 분할된 각 텍스트를 파일로 저장
        for i, chunk in enumerate(chunks):
            output_file_name = f"{os.path.splitext(file_name)[0]}_part_{i + 1}.txt"
            output_path = os.path.join(output_dir, output_file_name)
            with open(output_path, 'w', encoding='utf-8') as output_file:
                output_file.write(chunk)

        print(f"Processed {file_name}: {len(chunks)} parts created.")


# 사용 예시
input_directory = r'C:/Users/PC00/Desktop/AICOSS_doodle/TS2사회'
output_directory = r'C:/Users/PC00/Desktop/doodle_clone/cleaned-data/TS2사회'
os.makedirs(output_directory, exist_ok=True)  # 출력 디렉토리가 없으면 생성
clean_and_split_text(input_directory, output_directory)
