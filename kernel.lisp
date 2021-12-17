|>[#T ~>[[v1] [~>[[v2] [:v1]]]]]
|>[#F ~>[[v1] [~>[[v2] [:v2]]]]]
|>[nil ~>[[x] [:#T]]]
|>[id ~>[[x] [:x]]]
|>[cons ~>[[v1 v2] [~>[[cond][:>[:>[:cond :v1] :v2]]]]]]
|>[car ~>[[lst] [:>[:lst :#T]]]]
|>[cdr ~>[[lst] [:>[:lst :#F]]]]
|>[sel ~>[[cond v1 v2] [:>[:>[:cons :v1 :v2] :cond]]]]
|>[if ~>[[cond v1 v2] [::>[:sel :cond v1 v2]]]]
|>[empty ~>[[l] [:>[:l ~>[[head] [~>[[tail] [:#F]]]]]]]]
|>[concat ~>[[l1 l2][
    :>:>[:if 
        :>[:empty |>[x :>[:cdr :l1]]]
        [:cons :>[:car :l1] :l2]
        [:cons 
            :>[:car :l1]
            :>[:concat :x :l2]]]]]]

|>[plist ~>[[l][
    >:>[:if 
        :>[:empty :l]
        []
        [:>[:car :l] :>[:plist :>[:cdr :l]]]]]]]

|>[l1 :>[:cons 1 :nil]]
|>[l2 :>[:cons 2 :nil]]
|>[l3 :>[:cons 3 :l2]]
|>[loop ~>[[] [:>[:loop]]]]
|>[conc :>[:concat :l1 :l2]]
:>[:if :#F [:loop] [:id :conc]]

:>[:plist :>[:concat :l3 :l2]]
