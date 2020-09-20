#!/usr/bin/env python3
# pyre-strict

import re
from dataclasses import dataclass
from typing import IO, List, Tuple


@dataclass
class ErrorCode:
    type: str
    code: int


@dataclass
class Position:
    fileName: str
    line: int
    startColumn: int
    endColumn: int


@dataclass
class PositionedMessage:
    position: Position
    message: str


@dataclass
class Error:
    code: ErrorCode
    message: PositionedMessage
    reason: List[PositionedMessage]


class ParseException(Exception):
    pass


def make_error(
    errorCode: ErrorCode,
    position: Position,
    message: str,
    reasons: List[PositionedMessage],
) -> Error:
    return Error(errorCode, PositionedMessage(position, message), reasons)


def end_of_file(line: str) -> bool:
    return line == "" or line == "\n"


def parse_errors(output_file_name: str) -> List[Error]:
    with open(output_file_name) as output_file:
        try:
            return parse_error(output_file, True)
        except ParseException:
            try:
                return parse_error(output_file, False)
            except ParseException as ex:
                raise ParseException(f"at file {output_file_name}: {ex}")


def same_error(line: str, multiple_error_file: bool) -> bool:
    if multiple_error_file:
        return starts_with_space(line)
    else:
        return not end_of_file(line)


def parse_error(output_file: IO[str], multiple_error_file: bool) -> List[Error]:
    errors = []
    line = output_file.readline()
    while not end_of_file(line):
        if line == "No errors\n":
            return []
        position = parse_position(line)
        line = output_file.readline()
        if starts_with_space(line):
            # this is part of an hh_show, so ignore and continue
            while starts_with_space(line):
                line = output_file.readline()
            continue
        (message, errorCode) = parse_message_and_code(output_file, line)
        line = output_file.readline()
        reasons = []
        while same_error(line, multiple_error_file):
            reasonPos = parse_position(line)
            line = output_file.readline()
            (line, reasonMessage) = parse_message(output_file, line)
            reason = PositionedMessage(reasonPos, reasonMessage)
            reasons.append(reason)
        errors.append(make_error(errorCode, position, message, reasons))
    return errors


position_regex = r'^\s*File "(.*)", line (\d+), characters (\d+)-(\d+):(\[\d+\])?\n'


def parse_position(line: str) -> Position:
    match = re.match(position_regex, line)
    if match is None:
        raise ParseException(f"Could not parse position line: {line}")
    file = match.group(1)
    lineNum = int(match.group(2))
    startCol = int(match.group(3))
    endCol = int(match.group(4))
    return Position(file, lineNum, startCol, endCol)


def parse_message_and_code(file: IO[str], line: str) -> Tuple[str, ErrorCode]:
    message_chunks = []
    message_and_code_regex = r"^\s*(.*) \((.*)\[(\d+)\]\)\n"
    match = re.match(message_and_code_regex, line)
    while match is None:
        match = re.match(r"^\s*(.*)\n", line)
        if match is None:
            raise ParseException(f"Could not parse message line: {line}")
        message_line = match.group(1)
        message_chunks.append(message_line)
        line = file.readline()
        match = re.match(message_and_code_regex, line)
    assert match is not None, "should have raised exception above"
    message_line = match.group(1)
    type = match.group(2)
    code = int(match.group(3))
    message_chunks.append(message_line)
    message = "".join(message_chunks)
    return (message, ErrorCode(type, code))


def parse_message(file: IO[str], line: str) -> Tuple[str, str]:
    message_chunks = []
    message_regex = r"^\s*(.*)\n"

    match = re.match(message_regex, line)
    if match is None:
        raise ParseException(f"Could not parse message line: {line}")
    message_line = match.group(1)
    message_chunks.append(message_line)

    line = file.readline()
    match = re.match(position_regex, line)
    while match is None and not end_of_file(line):
        match = re.match(message_regex, line)
        if match is None:
            raise ParseException(f"Could not parse message line: {line}")
        message_line = match.group(1)
        message_chunks.append(message_line)

        line = file.readline()
        match = re.match(position_regex, line)
    message = "".join(message_chunks)
    return (line, message)


def starts_with_space(s: str) -> bool:
    return re.match(r"^\s", s) is not None


def sprint_error(error: Error) -> str:
    file = error.message.position.fileName
    line = error.message.position.line
    startCol = error.message.position.startColumn
    endCol = error.message.position.endColumn
    message = error.message.message
    type = error.code.type
    code = error.code.code
    out = [
        f"\033[91m{file}:{line}:{startCol},{endCol}:\033[0m "
        f"{message} ({type}[{code}])\n"
    ]

    for reason in error.reason:
        file = reason.position.fileName
        line = reason.position.line
        startCol = reason.position.startColumn
        endCol = reason.position.endColumn
        message = reason.message
        out.append(f"  \033[91m{file}:{line}:{startCol},{endCol}:\033[0m {message}\n")
    return "".join(out)


def sprint_errors(errors: List[Error]) -> str:
    if not errors:
        return "No errors\n"
    out = []
    for error in errors:
        out.append(sprint_error(error))
    return "".join(out)
