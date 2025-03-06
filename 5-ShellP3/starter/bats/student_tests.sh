#!/usr/bin/env bats

# File: student_tests.sh
# 
# Create your unit tests suit in this file

@test "Example: check ls runs without errors" {
    run ./dsh <<EOF                
ls
EOF


    [ "$status" -eq 0 ]
}

@test "Built-in command: exit terminates shell" {
    run ./dsh <<EOF
exit
EOF
    [ "$status" -eq 0 ]
}

@test "Built-in command: cd changes directory" {
    run ./dsh <<EOF
cd /
pwd
exit
EOF
    [ "$status" -eq 0 ]
    [[ "$output" == *"/"* ]]
}

@test "Built-in command: pwd shows current directory" {
    run ./dsh <<EOF
pwd
exit
EOF
    [ "$status" -eq 0 ]
    [ "${lines[0]}" != "" ]
}

@test "Built-in command: dragon prints dragon ASCII art" {
    run ./dsh <<EOF
dragon
exit
EOF
    [ "$status" -eq 0 ]
    [[ "$output" == *"DRAGON"* || "$output" == *"ASCII"* ]] 
}

@test "External command: ls executes successfully" {
    run ./dsh <<EOF
ls
exit
EOF
    [ "$status" -eq 0 ]
    [ "${lines[0]}" != "" ]
}

@test "External command with arguments: ls -l executes successfully" {
    run ./dsh <<EOF
ls -l
exit
EOF
    [ "$status" -eq 0 ]
    [[ "$output" == *"total"* ]]
}

@test "External command: echo prints correctly" {
    run ./dsh <<EOF
echo "Hello, world!"
exit
EOF
    [ "$status" -eq 0 ]
    [[ "$output" == *"Hello, world!"* ]]
}

@test "Empty input: should prompt again" {
    run ./dsh <<EOF

exit
EOF
    [ "$status" -eq 0 ]
}

@test "Whitespace input: should prompt again" {
    run ./dsh <<EOF
    
exit
EOF
    [ "$status" -eq 0 ]
}

# Test command argument handling

@test "Complex command: echo with quotes" {
    run ./dsh <<EOF
echo "Hello World"
exit
EOF
    [ "$status" -eq 0 ]
    [[ "$output" == *"Hello World"* ]]
}

@test "Command with environment variables: echo \$HOME" {
    run ./dsh <<EOF
echo "$HOME"
exit
EOF
    [ "$status" -eq 0 ]
    [[ "$output" == *"$HOME"* || "$output" == *"/home/"* ]]
}

@test "Command chaining with semicolon" {
    run ./dsh <<EOF
echo hello; echo world
exit
EOF
    [ "$status" -eq 0 ]
    [[ "$output" == *"hello"* ]]
    [[ "$output" == *"world"* ]]
}

@test "Redirection: echo to file" {
    run ./dsh <<EOF
echo "test content" > testfile.txt
cat testfile.txt
exit
EOF
    [ "$status" -eq 0 ]
    [[ "$output" == *"test content"* ]]
    rm -f testfile.txt
}

@test "Background process execution: sleep 1 &" {
    run ./dsh <<EOF
sleep 1 &
echo done
exit
EOF
    [ "$status" -eq 0 ]
    [[ "$output" == *"done"* ]]
}

@test "Simple pipe: echo to grep" {
    run ./dsh <<EOF
echo "Hello World" | grep World
exit
EOF
    [ "$status" -eq 0 ]
    [[ "$output" == *"Hello World"* ]]
}

@test "Multiple pipes: ls | grep test | wc -l" {
    run ./dsh <<EOF
ls | grep test | wc -l
exit
EOF
    [ "$status" -eq 0 ]
    [ "${lines[0]}" -ge 0 ]
}

@test "Pipe with spaces: echo and tr" {
    run ./dsh <<EOF
echo "Hello World" | tr ' ' '_'
exit
EOF
    [ "$status" -eq 0 ]
    [[ "$output" == *"Hello_World"* ]]
}

@test "Pipe with multiple commands: ls -l | grep test | sort" {
    run ./dsh <<EOF
ls -l | grep test | sort
exit
EOF
    [ "$status" -eq 0 ]
}

@test "Pipe with cat and wc: Count lines in a file" {
    echo -e "Line1\nLine2\nLine3" > testfile.txt
    run ./dsh <<EOF
cat testfile.txt | wc -l
exit
EOF
    [ "$status" -eq 0 ]
    [[ "$output" == *"3"* ]]
    rm -f testfile.txt
}

@test "Pipe failure handling: Invalid command in pipeline" {
    run ./dsh <<EOF
ls | non_existent_command | wc -l
exit
EOF
    [ "$status" -ne 0 ]
}


@test "Too many pipes: should return error" {
    run ./dsh <<EOF
echo hello | cat | cat | cat | cat | cat | cat | cat | cat | cat | cat
exit
EOF
    [ "$status" -eq 0 ]
    [[ "$output" == *"CMD_ERR_PIPE_LIMIT"* ]]
}

