#include <iostream>
#include <vector>
#include <sys/mman.h>
#include <cstring>
#include <stdexcept>
#include <unistd.h>

// x86-64 머신 코드 생성 (MOV + ADD + RET)
std::vector<unsigned char> generate_machine_code(int left, int right) {
    std::vector<unsigned char> machine_code = {
        0x48, 0xB8,  // MOV RAX, immediate(64bit)
        static_cast<unsigned char>(left), 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  
        0x48, 0xBB,  // MOV RBX, immediate(64bit)
        static_cast<unsigned char>(right), 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  
        0x48, 0x01, 0xD8,  // ADD RAX, RBX
        0xC3               // RET (return)
    };
    return machine_code;
}

// JIT 실행 (mmap + mprotect 사용)
int execute_machine_code(const std::vector<unsigned char>& machine_code) {
    size_t code_size = machine_code.size();

    // 1. 실행 가능한 메모리 할당 (먼저 PROT_WRITE 설정)
    void* exec_mem = mmap(NULL, code_size, PROT_READ | PROT_WRITE, 
                        MAP_ANONYMOUS | MAP_PRIVATE | MAP_JIT, -1, 0);
    if (exec_mem == MAP_FAILED) {
        throw std::runtime_error("메모리 할당 실패!");
    }

    // 2. 머신 코드 복사
    memcpy(exec_mem, machine_code.data(), code_size);

    // 3. 실행 권한 부여
    if (mprotect(exec_mem, code_size, PROT_READ | PROT_EXEC) == -1) {
        throw std::runtime_error("mprotect 실패!");
    }

    // 4. 함수 포인터로 변환 후 실행
    int (*func)() = (int (*)())exec_mem;
    return func();
}

int main() {
    std::cout << "JavaScript 코드 입력 (예: 5 + 8): ";
    int left, right;
    char op;
    std::cin >> left >> op >> right;

    if (op != '+') {
        std::cerr << "현재 + 연산만 지원합니다!" << std::endl;
        return 1;
    }

    // 1️⃣ AST → 머신 코드 변환
    std::vector<unsigned char> machine_code = generate_machine_code(left, right);
    
    // 2️⃣ 머신 코드 출력
    std::cout << "\n[x86-64 머신 코드 변환 결과]\n";
    for (unsigned char byte : machine_code) {
        printf("%02X ", byte);
    }
    std::cout << "\n";

    // 3️⃣ 실행
    int result = execute_machine_code(machine_code);
    std::cout << "\n[실행 결과] " << result << std::endl;

    return 0;
}