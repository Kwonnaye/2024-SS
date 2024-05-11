import os
import glob
import re

# 텍스트 파일이 있는 디렉토리
directory = "C:/Users/PC00/Desktop/AICOSS_doodle/TS2사회"


def format_text(text):
    # 첫 발언자 번호가 아닌 다른 "숫자:" 패턴을 찾아 개행 추가
    text = re.sub(r'(?<!^)(\d+:)', r'\n\1', text)

    # 불필요한 '~' 제거 (숫자 범위에 있는 '~'는 보존)
    text = re.sub(r'(\d+)~(\d+)', r'\1~\2', text)  # 숫자 범위 보존
    text = re.sub(r'~', '', text)  # 의미없는 '~' 제거

    # 두 개 이상의 연속된 개행을 한 개의 개행으로 축소
    text = re.sub(r'\n{2,}', '\n', text)

    return text


# 디렉토리 내의 _cleaned_cleaned.txt 파일을 처리
for file_name in glob.glob(os.path.join(directory, '*_cleaned_cleaned.txt')):
    try:
        with open(file_name, 'r', encoding='utf-8') as file:
            text = file.read()

        # 텍스트 포맷팅
        formatted_text = format_text(text)

        # 결과를 새 파일에 저장 (기존 파일명에 _formatted 추가)
        output_file_name = file_name.replace('_cleaned_cleaned.txt', '_cleaned_cleaned2.txt')
        with open(output_file_name, 'w', encoding='utf-8') as file:
            file.write(formatted_text)

        print(f"Formatted and saved: {output_file_name}")
    except Exception as e:
        print(f"Error processing {file_name}: {e}")

print("Formatting complete.")