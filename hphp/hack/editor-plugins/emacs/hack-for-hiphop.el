; Copyright (c) 2014, Facebook, Inc.
; All rights reserved.
;
; This source code is licensed under the BSD-style license found in the
; LICENSE file in the "hack" directory of this source tree. An additional grant
; of patent rights can be found in the PATENTS file in the same directory.

;; Setup the compile command
(setq compile-command "hh_client")
(global-set-key (kbd "M-RET") 'compile)

(if (boundp 'hack-for-hiphop-root)
  (setq compile-command (concat "hh_client --from emacs " hack-for-hiphop-root))

  ;; Compute the path to www whenever a php file is opened
  (add-hook 'php-mode-hook
    (lambda()
      (set (make-local-variable 'compile-command) (concat "hh_client "
        (locate-dominating-file (file-truename (buffer-name)) ".hhconfig")))))
)

;; Shortcut keys
(define-key input-decode-map "\e\eOA" [(meta up)])
(define-key input-decode-map "\e\eOB" [(meta down)])
(global-set-key [(meta up)] 'next-error)
(global-set-key [(meta down)] 'previous-error)
