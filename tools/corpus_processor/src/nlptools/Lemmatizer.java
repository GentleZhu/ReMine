package nlptools;

import com.google.common.base.Charsets;
import com.google.common.base.Joiner;
import com.google.common.io.Files;
import edu.stanford.nlp.ling.CoreAnnotations.LemmaAnnotation;
import edu.stanford.nlp.ling.CoreAnnotations.PartOfSpeechAnnotation;
import edu.stanford.nlp.ling.CoreAnnotations.SentencesAnnotation;
import edu.stanford.nlp.ling.CoreAnnotations.TokensAnnotation;
import edu.stanford.nlp.ling.CoreLabel;
import edu.stanford.nlp.pipeline.Annotation;
import edu.stanford.nlp.pipeline.StanfordCoreNLP;
import edu.stanford.nlp.util.CoreMap;

import java.io.File;
import java.io.IOException;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.*;

public class Lemmatizer {

    private StanfordCoreNLP pipeline;

    public Lemmatizer() throws IOException {
        // Create StanfordCoreNLP object properties, with POS tagging
        // (required for lemmatization), and lemmatization
        Properties props;
        props = new Properties();
        props.put("annotators", "tokenize, ssplit, pos, lemma");
        props.put("ssplit.boundaryTokenRegex", "\\.|[!?;:]+");

        /*
         * This is a pipeline that takes in a string and returns various analyzed linguistic forms.
         * The String is tokenized via a tokenizer (such as PTBTokenizerAnnotator),
         * and then other sequence model style annotation can be used to add things like lemmas,
         * POS tags, and named entities. These are returned as a list of CoreLabels.
         * Other analysis components build and store parse trees, dependency graphs, etc.
         *
         * This class is designed to apply multiple Annotators to an Annotation.
         * The idea is that you first build up the pipeline by adding Annotators,
         * and then you take the objects you wish to annotate and pass them in and
         * get in return a fully annotated object.
         *
         *  StanfordCoreNLP loads a lot of models, so you probably
         *  only want to do this once per execution
         */
        this.pipeline = new StanfordCoreNLP(props);
    }

    private List<String> lemmatize(String documentText) {
        List<String> lemmas = new LinkedList<>();
        // Create an empty Annotation just with the given text
        Annotation document = new Annotation(documentText);
        // run all Annotators on this text
        this.pipeline.annotate(document);
        // Iterate over all of the sentences found
        List<CoreMap> sentences = document.get(SentencesAnnotation.class);
        for (CoreMap sentence : sentences) {
            // Iterate over all tokens in a sentence
            for (CoreLabel token : sentence.get(TokensAnnotation.class)) {
                // Retrieve and add the lemma for each word into the
                // list of lemmas
                lemmas.add(token.get(LemmaAnnotation.class));
            }
        }
        return lemmas;
    }

    private List<List<List<String>>> lemmatizeBySentence(String documentText) {
        List<List<String>> lemmasBySentence = new ArrayList<>();
        List<List<String>> postagBySentence = new ArrayList<>();
        // Create an empty Annotation just with the given text
        Annotation document = new Annotation(documentText);
        // run all Annotators on this text
        this.pipeline.annotate(document);
        // Iterate over all of the sentences found
        List<CoreMap> sentences = document.get(SentencesAnnotation.class);
        for (CoreMap sentence : sentences) {
            // Iterate over all tokens in a sentence
            List<String> lemmaSentence = new ArrayList<>();
            List<String> postagSentence = new ArrayList<>();
            for (CoreLabel token : sentence.get(TokensAnnotation.class)) {
                // Retrieve and add the lemma for each word into the
                // list of lemmas
                String lemma = token.get(LemmaAnnotation.class).toLowerCase();
                String postag = token.get(PartOfSpeechAnnotation.class);
                lemmaSentence.add(lemma);
                postagSentence.add(postag);
            }
            lemmasBySentence.add(lemmaSentence);
            postagBySentence.add(postagSentence);
        }
        List<List<List<String>>> lemResult = new ArrayList<>();
        lemResult.add(lemmasBySentence);
        lemResult.add(postagBySentence);
        return lemResult;
    }

    public static void main(String[] args) throws IOException {
        if (args.length < 2) {
            System.out.println("Missing source text directory, output directory");
            return;
        }
        Path sourcePath = Paths.get(args[0]);
        Path outputPath = Paths.get(args[1]);
//        File outputFile = new File(args[2]);
        Lemmatizer lem = new Lemmatizer();
//        Multiset<String> lemmaOccurrences = HashMultiset.create();

        FileManager fileManager = new FileManager(sourcePath);
        List<Path> files = fileManager.getFiles();
        int totalFiles = files.size();
        int count = 0;
        for (Path path : files) {
            count += 1;
            System.out.println(count + "/" + totalFiles);

            String text = FileManager.readFile(path);
//            List<String> lemmas = lem.lemmatize(text);
            List<List<List<String>>> lemResult = lem.lemmatizeBySentence(text);
            List<List<String>> lemmasBySentence = lemResult.get(0);
            List<List<String>> postagBySentence = lemResult.get(1);
            List<String> lemmas = new ArrayList<>();
            List<String> postags = new ArrayList<>();
            for (int i = 0; i < lemmasBySentence.size(); i++) {
                List<String> lemmaSentence = lemmasBySentence.get(i);
                List<String> postagSentence = postagBySentence.get(i);
                lemmas.add(Joiner.on(' ').join(lemmaSentence));
                postags.add(Joiner.on(' ').join(postagSentence));
            }

            String lemmaLines = Joiner.on('\n').join(lemmas);
            String postagLines = Joiner.on('\n').join(postags);

            Path filename = path.getFileName();
            Path lemmaFilePath = outputPath.resolve(filename + ".lem");
            Files.write(lemmaLines, lemmaFilePath.toFile(), Charsets.UTF_8);
            Path postagFilePath = outputPath.resolve(filename + ".pos");
            Files.write(postagLines, postagFilePath.toFile(), Charsets.UTF_8);
        }
    }
}
