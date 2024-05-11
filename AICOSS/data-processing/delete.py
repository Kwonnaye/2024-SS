import os
import re

# 디렉토리 경로 설정
directory = r'C:/Users/PC00/Desktop/AICOSS_doodle/TS1경제'

# 파일명이 '_output_n.txt' 형식에 맞는지 확인하는 정규 표현식
pattern = re.compile(r'_output_\d+\.txt$')

# 지정된 디렉토리를 순회하면서 파일 검사
for filename in os.listdir(directory):
    if pattern.search(filename):
        full_path = os.path.join(directory, filename)
        print(f"Deleting {full_path}")  # 삭제할 파일명 출력
        os.remove(full_path)  # 파일 삭제

print("Deletion complete.")  # 삭제 완료 메시지
