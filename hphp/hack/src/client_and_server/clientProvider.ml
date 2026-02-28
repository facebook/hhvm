include
  (val if Injector_config.use_test_stubbing then
         (module TestClientProvider : ClientProvider_sig.S)
       else
         (module ServerClientProvider : ClientProvider_sig.S))
