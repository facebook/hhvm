<?hh

<<file:__EnableUnstableFeatures('module_references')>>

new module a {
    imports {
        global,
        x,
        y.*
    }
    exports {
        global,
        x,
        y.*
    }
}
