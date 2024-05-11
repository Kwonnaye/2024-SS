import os
import re

def extract_repeated_content(text):
    # 정규식을 사용하여 괄호 안의 내용을 찾습니다.
    pattern = r'\((.*?)\)'
    matches = re.findall(pattern, text)
    return matches if matches else None

def process_text(text):
    result = extract_repeated_content(text)
    if not result:
        return text

    # 문자열 공백 제거
    cleaned_list_result = [item.strip() for item in result]

    # 빈 문자열 제거
    result2 = re.split(r'\(|\)', text)
    result3 = [item.strip() for item in result2 if item.strip()]

    remove_words = cleaned_list_result[0::2]
    keep_words = cleaned_list_result[1::2]

    remove_words = list(set(remove_words))
    keep_words = list(set(keep_words))
    g = [i for i in result3 if i not in remove_words]

    # 결과 반환
    return " ".join(g)

def process_directory(directory):
    for filename in os.listdir(directory):
        if filename.endswith("_cleaned.txt"):
            base_name = filename[:-4]  # '.txt' 확장자 제거
            input_file_path = os.path.join(directory, filename)
            output_file_path = os.path.join(directory, f"{base_name}_cleaned.txt")

            # 파일 읽기
            with open(input_file_path, 'r', encoding='utf-8') as file:
                text = file.read()

            # 텍스트 처리
            processed_text = process_text(text)

            # 결과 저장
            with open(output_file_path, 'w', encoding='utf-8') as output_file:
                output_file.write(processed_text)

            print(f"Processed and saved: {output_file_path}")

# 사용 예시
directory = "C:/Users/PC00/Desktop/AICOSS_doodle/TS2사회"
process_directory(directory)