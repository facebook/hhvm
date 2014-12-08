<?php

// Various functions that accept resource arguments have different coerce modes

// False
var_dump('fclose', @fclose(false));
var_dump('pclose', @pclose(false));
var_dump('fseek', @fseek(false, 0));
var_dump('rewind', @rewind(false));
var_dump('ftell', @ftell(false));
var_dump('feof', @feof(false));
var_dump('fstat', @fstat(false));
var_dump('fread', @fread(false, 0));
var_dump('fgetc', @fgetc(false));
var_dump('fgets', @fgets(false));
var_dump('fgetss', @fgetss(false));
var_dump('fscanf', @fscanf(false));
var_dump('fpassthru', @fpassthru(false));
var_dump('fwrite', @fwrite(false, ''));
var_dump('fputs', @fputs(false, ''));
var_dump('fprintf', @fprintf(false, ''));
var_dump('vfprintf', @vfprintf(false, '', array()));
var_dump('fflush', @fflush(false));
var_dump('ftruncate', @ftruncate(false));

// NULL
var_dump('fputcsv', @fputcsv(false, array()));
var_dump('fgetcsv', @fgetcsv(false));
var_dump('flock', @flock(false, 0));
var_dump('readdir', @readdir(false));
var_dump('rewinddir', @rewinddir(false));
var_dump('closedir', @closedir(false));
