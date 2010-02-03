
;; Better HDF read/write experience in emacs (9/25/09 hzhao@facebook.com)

(require 'font-lock)

(defvar hdf-mode-hook nil)
(add-to-list 'auto-mode-alist '("\\.hdf\\'" . hdf-mode))

(defvar hdf-indent-level 2
  "Defines 2 spaces for HDF indentation.")

;; syntax coloring
;; http://www.gnu.org/software/emacs/elisp/html_node/Faces-for-Font-Lock.html
(defconst hdf-font-lock-keywords
  (list
   '("^[ \t]*\\([\\#\\-]include\\)[ \t]+\\(.*\\)"
     (1 font-lock-keyword-face)
     (2 font-lock-string-face))              ;; include
   '("^[ \t]*#.*$" . font-lock-comment-face) ;; comments
   '("^[ \t]*\\([a-z0-9_\\.\\*]+\\)[ \t]*\\(!=\\)[ \t]*\\(.*\\)"
     (1 font-lock-variable-name-face)
     (2 font-lock-function-name-face))       ;; shell commands
   '("^[ \t]*\\([a-z0-9_\\.\\*]+\\)[ \t]*\\(:=\\)[ \t]*\\([a-z0-9\\.]+\\)[ \t]*$"
     (1 font-lock-variable-name-face)
     (2 font-lock-function-name-face)
     (3 font-lock-variable-name-face))       ;; node copying
   '("^[ \t]*\\([a-z0-9_\\.\\*]+\\)[ \t]*=[ \t]*\\(true\\|false\\|yes\\|no\\|on\\|off\\)[ \t]*$"
     (1 font-lock-variable-name-face)
     (2 font-lock-keyword-face))             ;; booleans
   '("^[ \t]*\\([a-z0-9_\\.\\*]+\\)[ \t]*=[ \t]*\\([0-9]+\\)[ \t]*$"
     (1 font-lock-variable-name-face)
     (2 font-lock-constant-face))            ;; numbers
   '("^[ \t]*\\([a-z0-9_\\.\\*]+\\)[ \t]*=[ \t]*\\(.*\\)"
     (1 font-lock-variable-name-face))       ;; strings
   '("^[ \t]*\\([a-z0-9_\\.\\*]+\\)[ \t]*[{=][ \t]*$"
     (1 font-lock-variable-name-face))       ;; nodes
   '("^[ \t]*\\([a-z0-9_\\.\\*]+\\)[ \t]*\\(:\\)[ \t]*\\([a-z0-9\\.]+\\)[ \t]*$"
     (1 font-lock-variable-name-face)
     (2 font-lock-function-name-face)
     (3 font-lock-variable-name-face))       ;; node aliases
   '("^[ \t]*\\(@\\)\\([a-z0-9_\\.]+\\)[ \t]*$"
     (1 font-lock-function-name-face)
     (2 font-lock-variable-name-face))       ;; node inheritance
   )
  "Hdf Keywords")

;; indentation
(defun hdf-indent-line ()
  "Indent current line as HDF code."
  (interactive)
  (beginning-of-line)
  (if (bobp)
      (indent-line-to 0)
    (progn
      (if (looking-at "^[ \t]*}")
          (save-excursion
            (forward-line -1)
            (while (and (not (bobp)) (looking-at "^[ \t]*$"))
              (forward-line -1))
            (if (looking-at "^[ \t]*\\([a-z0-9_\\.\\*]+\\)[ \t]*{")
                (setq cur-indent (current-indentation))
              (progn
                (setq cur-indent (- (current-indentation) hdf-indent-level))
                (if (< cur-indent 0)
                    (indent-line-to 0)))))
        (save-excursion
          (forward-line -1)
          (while (and (not (bobp)) (looking-at "^[ \t]*$"))
            (forward-line -1))
          (if (looking-at "^[ \t]*\\([a-z0-9_\\.\\*]+\\)[ \t]*{")
              (setq cur-indent (+ (current-indentation) hdf-indent-level))
            (setq cur-indent (current-indentation)))))
      (if cur-indent
          (indent-line-to cur-indent)
        (indent-line-to 0)))))

(defun hdf-mode ()
  "Mode for editing HDF files"
  (interactive)
  (kill-all-local-variables)
  (set (make-local-variable 'font-lock-defaults)
       '(hdf-font-lock-keywords nil, 1))
  (setq major-mode 'hdf-mode)
  (setq mode-name "HDF")
  (run-hooks 'hdf-mode-hook)
  (set (make-local-variable 'indent-line-function) 'hdf-indent-line)
  )
(provide 'hdf-mode)
