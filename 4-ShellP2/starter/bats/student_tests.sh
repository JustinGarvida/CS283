#!/usr/bin/env bats

# File: student_tests.sh

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

@test "Complex command: echo with quotes" {
    run ./dsh <<EOF
echo "Hello World"
exit
EOF
    [ "$status" -eq 0 ]
    [[ "$output" == *"Hello World"* ]]
}

@test "Multiple commands executed sequentially" {
    run ./dsh <<EOF
pwd
cd ..
pwd
exit
EOF
    [ "$status" -eq 0 ]
    [ "${#lines[@]}" -ge 2 ]
    [ "${lines[0]}" != "${lines[1]}" ]
}

@test "Built-in command: invalid directory with cd" {
    run ./dsh <<EOF
cd nonexistent_directory
exit
EOF
    [ "$status" -eq 0 ]
    [[ "$output" == *"No such file or directory"* ]]
}

@test "External command: invalid command returns error" {
    run ./dsh <<EOF
invalidcommand
exit
EOF
    [ "$status" -eq 0 ]
    [[ "$output" == *"No such file or directory"* || "$output" == "" ]]
}

@test "Built-in command: pwd shows current directory" {
    run ./dsh <<EOF
pwd
exit
EOF
    [ "$status" -eq 0 ]
    [ "${lines[0]}" != "" ]
}

@test "Command with special characters: echo \"$HOME\"" {
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

@test "Built-in command: dragon prints dragon" {
    run ./dsh <<EOF
dragon
exit
EOF
    [ "$status" -eq 0 ]
    [[ "$output" == *"dragon"* ]]
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
