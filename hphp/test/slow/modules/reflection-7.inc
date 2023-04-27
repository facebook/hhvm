<?hh

<<file:__EnableUnstableFeatures('module_references')>>

new module a {
    exports {
        global,
        x,
        y.*
    }
    imports {
        global,
        x,
        y.*
    }
}
