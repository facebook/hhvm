# Terms and Definitions
For the purposes of this document, the following terms and definitions
apply:

<pre>

  <b>argument:</b>
    a value passed to a function, that is intended to map to a
    corresponding parameter.
      
  <b>behavior:</b>
    external appearance or action.

  <b>behavior, implementation-defined:</b>
    behavior specific to an implementation, where that implementation
    must document that behavior.

  <b>behavior, undefined:</b>
    behavior which is not guaranteed to
    produce any specific result. Usually follows an erroneous program
    construct or data.

  <b>behavior, unspecified:</b>
    behavior for which this specification provides no requirements.

  <b>case-preserved:</b>
    a construct which is case-insensitive upon declaration, but
    case-sensitive upon subsequent usage.

  <b>constraint:</b>
    restriction, either syntactic or semantic, on how language elements
    can be used.

  <b>error, fatal:</b>
    a translation or runtime condition from which the translator or
    engine cannot recover.

  <b>error, fatal, catchable:</b>
    a fatal error that can be caught by a user-defined handler.

  <b>error, non-fatal:</b>
    an error that is not fatal.

  <b>lvalue:</b>
    an expression that designates a memory location having a type.

  <b>lvalue, modifiable:</b>
    an lvalue whose value can be changed.

  <b>lvalue, non-modifiable:</b>
    an lvalue whose value cannot be changed.

  <b>parameter:</b>
    a variable declared in the parameter list of a function that is
    intended to map to a corresponding argument in a call to that
    function.

  <b>Hack Run-Time Engine:</b>
    the machinery that executes a Hack program. Referred to as *the
    Engine* throughout this specification.

  <b>value:</b>
    precise meaning of the contents of a memory location when
    interpreted as having a specific type.
</pre>

Other terms are defined throughout this specification, as needed, with
the first usage being typeset *like this*.
