import os

def split_text_file(input_file, output_directory, max_length=5000):
    with open(input_file, 'r', encoding='utf-8') as f:
        text = f.read()
        sections = text.split(':')
        current_length = 0
        current_text = ''
        file_count = 1
        # Get the base name of the input file
        base_name = os.path.basename(input_file).split('.')[0]
        for section in sections:
            if current_length + len(section) > max_length:
                with open(os.path.join(output_directory, f'{base_name}_output_{file_count}.txt'), 'w', encoding='utf-8') as output_file:
                    output_file.write(current_text)
                current_text = '참석자:' + section
                current_length = len(current_text)
                file_count += 1
            else:
                current_text += '참석자:' + section
                current_length += len('참석자:' + section)
        if current_text:
            with open(os.path.join(output_directory, f'{base_name}_output_{file_count}.txt'), 'w', encoding='utf-8') as output_file:
                output_file.write(current_text)

def process_multiple_txt_files(input_directory, output_directory):
    for filename in os.listdir(input_directory):
        if filename.endswith('_cleaned2.txt'):
            split_text_file(os.path.join(input_directory, filename), output_directory)

# Call the function to process multiple txt files
# Note: Replace 'input_directory' with the actual directory containing the txt files
# Replace 'output_directory' with the actual directory where you want to save the output files
directory = "C:/Users/PC00/Desktop/AICOSS_doodle/TS1경제"
process_multiple_txt_files(directory, directory)

# Print a success message
print("각 파일당 5000자 이하로 쪼개서 출력이 됬습니다^0^")